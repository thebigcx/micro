#include <micro/sys.h>
#include <micro/types.h>
#include <micro/task.h>
#include <micro/vfs.h>
#include <arch/reg.h>
#include <arch/cpu.h>
#include <micro/thread.h>
#include <micro/sched.h>
#include <micro/errno.h>
#include <micro/fs.h>
#include <micro/mman.h>
#include <micro/stdlib.h>
#include <micro/heap.h>
#include <micro/wait.h>

int is_valid_ptr(const void* ptr)
{
    if (!ptr || (uintptr_t)ptr > 0x8000000000) return 0;
    return mmu_virt2phys(task_curr()->vm_map, (uintptr_t)ptr); // Returns 0 if a page is non-present
}

#define PTRVALID(ptr) { if (!is_valid_ptr(ptr)) return -EFAULT; }

static int sys_open(const char* path, uint32_t flags, mode_t mode)
{
    PTRVALID(path);
    
    // Must specify an access mode
    //if (!(flags & 3)) return -EINVAL;

    struct task* task = task_curr();

    char* canon = vfs_mkcanon(path, task->workd);
    struct file* file = vfs_resolve(canon);

    if (!file)
    {
        if (!(flags & O_CREAT)) return -ENOENT;
        vfs_mkfile(canon);
        file = vfs_resolve(canon);
    }

    kfree(canon);

    list_push_back(&task->fds, vfs_open(file));
    return task->fds.size - 1;
}

static int sys_close(int fdno)
{
    struct task* task = task_curr();
    if (fdno < 0 || (size_t)fdno >= task->fds.size) return -EBADF;

    vfs_close(list_remove(&task->fds, fdno));

    return 0;
}

static ssize_t sys_read(int fdno, void* buf, size_t size)
{
    PTRVALID(buf);

    struct task* task = task_curr();
    if (fdno < 0 || (size_t)fdno >= task->fds.size) return -EBADF;

    struct fd* fd = list_get(&task->fds, fdno);
    ssize_t ret = vfs_read(fd->filp, buf, fd->off, size);
    fd->off += size;
    return ret;
}

static ssize_t sys_write(int fdno, const void* buf, size_t size)
{
    PTRVALID(buf);

    struct task* task = task_curr();
    if (fdno < 0 || (size_t)fdno >= task->fds.size) return -EBADF;

    struct fd* fd = list_get(&task->fds, fdno);
    ssize_t ret = vfs_write(fd->filp, buf, fd->off, size);
    fd->off += size;
    return ret;
}

static int sys_fork()
{
    struct thread* calling = thread_curr();

    struct task* child = task_clone(task_curr(), calling);

    memcpy(&((struct thread*)child->threads.head->data)->regs, &calling->syscall_regs, sizeof(struct regs));
    ((struct thread*)child->threads.head->data)->regs.rax = 0; // return 0 to the child task

    sched_start(child);

    return child->id;
}

static int sys_execve(const char* path, const char* argv[], const char* envp[])
{
    PTRVALID(path);
    PTRVALID(argv);
    PTRVALID(envp);

    char* canon = vfs_mkcanon(path, task_curr()->workd);
    struct file* file = vfs_resolve(canon);
    kfree(canon);

    if (!file) return -ENOENT;
    if (file->flags == FL_DIR) return -EISDIR;

    char* argv_copy[16];
    size_t argc = 0;
    while (argv[argc] != NULL)
    {
        PTRVALID(argv[argc]);

        argv_copy[argc] = kmalloc(strlen(argv[argc]) + 1);
        strcpy(argv_copy[argc], argv[argc]);
        argc++;
    }
    argv_copy[argc] = NULL;

    // TODO: copy envp
    task_execve(task_curr(), canon, (const char**)argv_copy, envp);
    sched_yield();
    return -1;
}

static int sys_exit(int stat)
{
    task_exit(stat);
    return -1; // Should be unreachable
}

static int sys_kill(int pid, int sig)
{
    if (sig < 0 || sig > 32) return -EINVAL;

    // TODO: impl proper
    struct task* task = sched_task_fromid(pid);
    if (!task) return -ESRCH;
    task_send(task, sig);

    if (task_curr() == task) sched_yield();

    return 0;
}

static int sys_getpid()
{
    return task_curr()->id;
}

static int sys_access(const char* pathname, int mode)
{
    PTRVALID(pathname);

    char* canon = vfs_mkcanon(pathname, task_curr()->workd);
    int ret = vfs_access(canon, mode);
    kfree(canon);

    return ret;
}

static off_t sys_lseek(int fdno, off_t offset, int whence)
{
    struct task* task = task_curr();
    if (fdno < 0 || (size_t)fdno >= task->fds.size) return -EBADF;

    struct fd* fd = list_get(&task->fds, fdno);

    switch (whence)
    {
        case SEEK_SET:
            fd->off = offset;
            return fd->off;
        case SEEK_END:
            fd->off = fd->filp->size + offset;
            return fd->off;
        case SEEK_CUR:
            fd->off += offset;
            return fd->off;
    }

    return -EINVAL;
}

// TODO: add support for thread blocking

static int sys_waitpid(int pid, int* wstatus, int options)
{
    PTRVALID(wstatus);

    if (options > (WNOHANG | WUNTRACED) || options < 0) return -EINVAL;

    struct task* task = sched_task_fromid(pid);
    if (!task || task->parent != task_curr()) return -ECHILD;

    if (!task->dead && options & WNOHANG) return 0;

    while (!task->dead) sched_yield(); // FIXME: this is a pretty poor way of mimicking thread blocking

    task_delete(task);

    return pid;
}

// TODO: currently only supports fixed anonymous mappings
static void* sys_mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    if (!length) return (void*)-EINVAL;
    if (!(flags & MAP_SHARED) && !(flags & MAP_PRIVATE)) return (void*)-EINVAL;
    // TEMPORARY
    if (!(flags & MAP_ANONYMOUS) || !(flags & MAP_FIXED)) return (void*)-EINVAL;

    unsigned int mmu_flags = PAGE_PR;
    mmu_flags |= prot & PROT_WRITE ? PAGE_RW : 0;

    for (uintptr_t i = (uintptr_t)addr; i < (uintptr_t)addr + length; i += PAGE4K)
    {
        mmu_map(task_curr()->vm_map, i, mmu_alloc_phys(), mmu_flags);
    }

    return addr;
}

static int sys_munmap(void* addr, size_t length)
{
    if (!length) return -EINVAL;
    if ((uintptr_t)addr % PAGE4K != 0) return -EINVAL;

    // TODO: unmap the physical blocks if anonymous
    return 0;
}

// TODO: add PATH_MAX macro
static int sys_chdir(const char* path)
{
    PTRVALID(path);

    struct task* task = task_curr();
    char* new = vfs_mkcanon(path, task->workd);

    struct file* dir = vfs_resolve(new);
    
    if (!dir) return -ENOENT;
    if (dir->flags != FL_DIR && dir->flags != FL_MNTPT) return -ENOTDIR;

    strcpy(task->workd, new);

    kfree(dir);
    kfree(new);

    return 0;
}

static char* sys_getcwd(char* buf, size_t size)
{
    if (!is_valid_ptr(buf)) return (char*)-EFAULT;
    if (size == 0) return (char*)-EINVAL;

    struct task* task = task_curr();
    if (size < strlen(task->workd) + 1) return (char*)-ERANGE;

    strcpy(buf, task->workd);

    return buf;
}

static int sys_readdir(int fdno, size_t idx, struct dirent* dirent)
{
    PTRVALID(dirent);

    struct task* task = task_curr();
    if (fdno < 0 || (size_t)fdno >= task->fds.size) return -EBADF;

    struct fd* fd = list_get(&task->fds, fdno);

    if (fd->filp->flags != FL_DIR && fd->filp->flags != FL_MNTPT) return -ENOTDIR;

    return vfs_readdir(fd->filp, idx, dirent);
}

static int sys_mkdir(const char* path)
{
    PTRVALID(path);

    struct task* task = task_curr();
    char* canon = vfs_mkcanon(path, task->workd);

    if (canon[0] == 0) return -ENOENT;
    if (sys_access(canon, F_OK) == 0) return -EEXIST;

    vfs_mkdir(canon);

    return 0;
}

static int sys_ioctl(int fdno, unsigned long req, void* argp)
{
    PTRVALID(argp);

    struct task* task = task_curr();
    if (fdno < 0 || (size_t)fdno >= task->fds.size) return -EBADF;

    struct fd* fd = list_get(&task->fds, fdno);

    if (fd->filp->flags == FL_FILE || fd->filp->flags == FL_DIR) return -ENOTTY;

    return vfs_ioctl(fd->filp, req, argp);
}

typedef uintptr_t (*syscall_t)();

static uintptr_t syscalls[] =
{
    (uintptr_t)sys_open,
    (uintptr_t)sys_close,
    (uintptr_t)sys_read,
    (uintptr_t)sys_write,
    (uintptr_t)sys_fork,
    (uintptr_t)sys_execve,
    (uintptr_t)sys_exit,
    (uintptr_t)sys_kill,
    (uintptr_t)sys_getpid,
    (uintptr_t)sys_access,
    (uintptr_t)sys_lseek,
    (uintptr_t)sys_waitpid,
    (uintptr_t)sys_mmap,
    (uintptr_t)sys_munmap,
    (uintptr_t)sys_chdir,
    (uintptr_t)sys_getcwd,
    (uintptr_t)sys_readdir,
    (uintptr_t)sys_mkdir,
    (uintptr_t)sys_ioctl
};

void syscall_handler(struct regs* r)
{
    uintptr_t n = arch_syscall_num(r);
    if (n >= sizeof(syscalls) / sizeof(syscall_t))
    {
        arch_syscall_ret(r, -ENOSYS);
        return;
    }

    thread_curr()->syscall_regs = *r;
    syscall_t sc = (syscall_t)syscalls[n];
    arch_syscall_ret(r, sc(ARCH_SCARG0(r),
                           ARCH_SCARG1(r),
                           ARCH_SCARG2(r),
                           ARCH_SCARG3(r),
                           ARCH_SCARG4(r),
                           ARCH_SCARG5(r)));
}

void sys_init()
{
    idt_set_handler(128, syscall_handler);
}