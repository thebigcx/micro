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

int is_valid_ptr(uintptr_t ptr)
{
    if (!ptr || ptr > 0x8000000000) return 0;
    return mmu_virt2phys(task_curr()->vm_map, ptr); // Returns 0 if a page is non-present
}

#define PTRVALID(ptr) { if (!is_valid_ptr(ptr)) return -EFAULT; }

// TODO: API folder

static int sys_open(const char* path, uint32_t flags)
{
    PTRVALID(path);
    
    struct task* task = task_curr();

    struct file* file = vfs_resolve(path); // TODO: canonicalize
    if (!file) return -ENOENT;

    list_push_back(&task->fds, vfs_open(file));
    return task->fds.size - 1;
}

static int sys_close(int fd)
{
    // TODO: remove list element at
}

static ssize_t sys_read(int fdno, void* buf, size_t size)
{
    PTRVALID(buf);

    struct task* task = task_curr();
    if (fdno < 0 || fdno >= task->fds.size) return -EBADF;

    struct fd* fd = list_get(&task->fds, fdno);
    ssize_t ret = vfs_read(fd->filp, buf, fd->off, size);
    fd->off += size;
    return ret;
}

static ssize_t sys_write(int fdno, const void* buf, size_t size)
{
    PTRVALID(buf);

    struct task* task = task_curr();
    if (fdno < 0 || fdno >= task->fds.size) return -EBADF;

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

    struct file* file = vfs_resolve(path); // TODO: canonicalize
    if (!file) return -ENOENT;

    const char* argv_copy[16];
    size_t argc = 0;
    while (argv[argc] != NULL)
    {
        PTRVALID(argv[argc]);

        argv_copy[argc] = kmalloc(strlen(argv[argc]) + 1);
        strcpy(argv_copy[argc], argv[argc]);
        argc++;
    }
    argv_copy[argc] = NULL;

    task_execve(task_curr(), path, argv_copy, envp);
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

    return vfs_access(pathname, mode);
}

static off_t sys_lseek(int fdno, off_t offset, int whence)
{
    struct task* task = task_curr();
    if (fdno < 0 || fdno >= task->fds.size) return -EBADF;

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

typedef uintptr_t (*syscall_t)();

static syscall_t syscalls[] =
{
    sys_open,
    sys_close,
    sys_read,
    sys_write,
    sys_fork,
    sys_execve,
    sys_exit,
    sys_kill,
    sys_getpid,
    sys_access,
    sys_lseek
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
    syscall_t sc = syscalls[n];
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