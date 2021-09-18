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
#include <micro/fcntl.h>
#include <micro/mman.h>
#include <micro/stdlib.h>
#include <micro/heap.h>
#include <micro/wait.h>
#include <micro/module.h>
#include <micro/tty.h>

// TODO: move syscalls into their own files

int is_valid_ptr(const void* ptr)
{
    if (!ptr || (uintptr_t)ptr > 0x8000000000) return 0;
    return mmu_virt2phys(task_curr()->vm_map, (uintptr_t)ptr); // Returns 0 if a page is non-present
}

#define PTRVALID(ptr) { if (!is_valid_ptr(ptr)) return -EFAULT; }

static int is_valid_range_fd(int fd)
{
    return fd >= 0 && fd < FD_MAX;
}

static int is_valid_fd(struct task* task, int fd)
{
    return is_valid_range_fd(fd) && task->fds[fd];
}

#define FDVALID(fd) { if (!is_valid_fd(task_curr(), fd)) return -EBADF; }
#define FDRANGEVALID(fd) { if (!is_valid_range_fd(fd)) return -EBADF; }

// TODO: make opening files better and more organized
static int sys_open(const char* path, uint32_t flags, mode_t mode)
{
    PTRVALID(path);

    // Must specify an access mode
    if (!(flags & 3)) return -EINVAL;

    struct task* task = task_curr();

    char* canon = vfs_mkcanon(path, task->workd);
    struct file* file = kmalloc(sizeof(struct file));
    int e = vfs_resolve(canon, file);

    if (e == -ENOENT)
    {
        if (!(flags & O_CREAT)) return -ENOENT;
        // TODO: this hshoudl be better
        vfs_mkfile(canon);
        vfs_resolve(canon, file);
    }
    else
    {
        if ((flags & O_CREAT) && (flags & O_EXCL)) return -EEXIST;
    }

    kfree(canon);

    for (unsigned int i = 0; i < FD_MAX; i++)
    {
        if (!task->fds[i])
        {
            task->fds[i] = vfs_open(file, flags, mode);
            return i;
        }
    }

    return -EMFILE;
}

static int sys_close(int fd)
{
    FDVALID(fd);

    struct task* task = task_curr();

    vfs_close(task->fds[fd]);
    task->fds[fd] = NULL;

    return 0;
}

static ssize_t sys_read(int fdno, void* buf, size_t size)
{
    FDVALID(fdno);
    PTRVALID(buf);

    struct task* task = task_curr();
    
    struct fd* fd = task->fds[fdno];
    ssize_t ret = vfs_read(fd->filp, buf, fd->off, size);
    fd->off += size;

    return ret;
}

static ssize_t sys_write(int fdno, const void* buf, size_t size)
{
    PTRVALID(buf);
    FDVALID(fdno);

    struct task* task = task_curr();
    
    struct fd* fd = task->fds[fdno];
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

    struct file* file = kmalloc(sizeof(struct file));
    int e = vfs_resolve(canon, file);
    
    kfree(canon);

    if (e) return e;
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

    char* env_copy[32];
    size_t envc = 0;
    while (env_copy[envc] != NULL)
    {
        PTRVALID(envp[envc]);

        env_copy[envc] = kmalloc(strlen(envp[envc]) + 1);
        strcpy(env_copy[envc], envp[envc]);
        envc++;
    }
    env_copy[envc] = NULL;

    // TODO: copy envp
    task_execve(task_curr(), canon, (const char**)argv_copy, (const char**)env_copy);
    sched_yield();
    return -1;
}

static int sys_exit(int stat)
{
    task_exit((stat & 0xff) << 8);
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
    FDVALID(fdno);

    struct fd* fd = task_curr()->fds[fdno];

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
    if (options > (WNOHANG | WUNTRACED) || options < 0) return -EINVAL;
    if (pid == INT_MIN) return -ESRCH;

    struct task* task = sched_task_fromid(pid);
    if (!task || task->parent != task_curr()) return -ECHILD;

    if (!task->dead && options & WNOHANG) return 0;

    while (!task->dead) sched_yield(); // FIXME: this is a pretty poor way of mimicking thread blocking

    if (wstatus) *wstatus = task->status;

    task_delete(task);

    return pid;
}

// TODO: add support for shared memory
// TODO: use the 'offset' parameter
static void* sys_mmap(void* addr, size_t length, int prot, int flags, int fdno, off_t offset)
{
    if (!length) return (void*)-EINVAL;
    if (!(flags & MAP_SHARED) && !(flags & MAP_PRIVATE)) return (void*)-EINVAL;

    if (flags & MAP_ANONYMOUS)
    {
        // TEMP
        if (flags & MAP_SHARED) return (void*)-EINVAL;

        unsigned int mmu_flags = PAGE_PR;
        mmu_flags |= prot & PROT_WRITE ? PAGE_RW : 0;

        for (uintptr_t i = (uintptr_t)addr; i < (uintptr_t)addr + length; i += PAGE4K)
        {
            mmu_map(task_curr()->vm_map, i, mmu_alloc_phys(), mmu_flags);
        }
    }
    else
    {
        // TEMP
        if (flags & MAP_SHARED) return (void*)-EINVAL;

        FDVALID(fdno);

        struct fd* fd = task_curr()->fds[fdno];

        if (fd->filp->ops.mmap)
        {
            struct vm_area area =
            {
                .begin = (uintptr_t)addr,
                .end   = (uintptr_t)addr + length
            };
            vfs_mmap(fd->filp, &area);
        }
        else
        {
            if (fd->filp->flags != FL_FILE) return (void*)-EACCES;

            unsigned int mmu_flags = PAGE_PR;
            mmu_flags |= prot & PROT_WRITE ? PAGE_RW : 0;

            for (uintptr_t i = (uintptr_t)addr; i < (uintptr_t)addr + length; i += PAGE4K)
            {
                mmu_map(task_curr()->vm_map, i, mmu_alloc_phys(), mmu_flags);
            }

            vfs_read(fd->filp, addr, 0, fd->filp->size);
        }
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

    struct file* dir = kmalloc(sizeof(struct file));
    int e = vfs_resolve(new, dir);
    
    if (e) return e;
    if (dir->flags != FL_DIR) return -ENOTDIR;

    strcpy(task->workd, new);

    kfree(dir);
    kfree(new);

    return 0;
}

static char* sys_getcwd(char* buf, size_t size)
{
    PTRVALID(buf);
    
    if (size == 0) return (char*)-EINVAL;

    struct task* task = task_curr();
    if (size < strlen(task->workd) + 1) return (char*)-ERANGE;

    strcpy(buf, task->workd);

    return buf;
}

static ssize_t sys_getdents(int fdno, struct dirent* dirp, size_t n)
{
    PTRVALID(dirp);
    FDVALID(fdno);

    struct fd* fd = task_curr()->fds[fdno];

    if (fd->filp->flags != FL_DIR) return -ENOTDIR;

    return vfs_getdents(fd->filp, fd->off++, n, dirp);
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
    FDVALID(fdno);
    PTRVALID(argp);
    
    struct fd* fd = task_curr()->fds[fdno];

    if (fd->filp->flags == FL_FILE || fd->filp->flags == FL_DIR) return -ENOTTY;

    return vfs_ioctl(fd->filp, req, argp);
}

// TODO: implement
static int sys_time(time_t* time)
{
    PTRVALID(time);
    *time = 0;
    return 0;
}

static int sys_dup(int oldfd)
{
    FDVALID(oldfd);
    
    struct task* task = task_curr();

    for (unsigned int i = 0; i < FD_MAX; i++)
    {
        if (!task->fds[i])
        {
            task->fds[i] = oldfd;
            return i;
        }
    }

    return -EMFILE;
}

static int sys_dup2(int oldfd, int newfd)
{
    FDVALID(oldfd);
    FDRANGEVALID(newfd);

    struct task* task = task_curr();
    
    task->fds[newfd] = task->fds[oldfd];
    return newfd;
}

static int sys_insmod(void* data, size_t len)
{
    PTRVALID(data);
    return module_load(data, len);
}

static int sys_rmmod(const char* name)
{
    PTRVALID(name);
    return module_free(name);
}

static int sys_mount(const char* src, const char* dst,
                     const char* fs, unsigned long flags,
                     const void* data)
{
    PTRVALID(src);
    PTRVALID(dst);
    PTRVALID(fs);

    // TODO: vfs_mount_fs should return an error code
    return vfs_mount_fs(src, dst, fs, data);
}

static int sys_umount(const char* target)
{
    PTRVALID(target);
    return vfs_umount_fs(target);
}

static ssize_t sys_pread(int fdno, void* buf, size_t size, off_t off)
{
    FDVALID(fdno);
    PTRVALID(buf);

    struct task* task = task_curr();
    ssize_t ret = vfs_read(task->fds[fdno]->filp, buf, off, size);

    return ret;
}

static ssize_t sys_pwrite(int fdno, const void* buf, size_t size, off_t off)
{
    PTRVALID(buf);
    FDVALID(fdno);

    struct task* task = task_curr();
    ssize_t ret = vfs_write(task->fds[fdno]->filp, buf, off, size);

    return ret;
}

static int sys_ptsname(int fdno, char* buf, size_t n)
{
    PTRVALID(buf);
    FDVALID(fdno);

    struct fd* fd = task_curr()->fds[fdno];

    // TODO: struct file should hold flags like MASTER_PTY, DEVICE, etc (for isatty())
    // THIS IS DANGEROUS
    struct pt* pt = fd->filp->device;


    if (strlen("/dev/pts/") + strlen(pt->pts->name) >= n) return -ERANGE;

    strcpy(buf, "/dev/pts/");
    strcpy(buf + strlen(buf), pt->pts->name);

    return 0;
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
    (uintptr_t)sys_getdents,
    (uintptr_t)sys_mkdir,
    (uintptr_t)sys_ioctl,
    (uintptr_t)sys_time,
    (uintptr_t)sys_dup,
    (uintptr_t)sys_dup2,
    (uintptr_t)sys_insmod,
    (uintptr_t)sys_rmmod,
    (uintptr_t)sys_mount,
    (uintptr_t)sys_umount,
    (uintptr_t)sys_pread,
    (uintptr_t)sys_pwrite,
    (uintptr_t)sys_ptsname
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