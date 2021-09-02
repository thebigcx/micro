#include <micro/sys.h>
#include <micro/types.h>
#include <micro/task.h>
#include <micro/vfs.h>
#include <arch/reg.h>
#include <arch/cpu.h>
#include <micro/thread.h>
#include <micro/sched.h>

// TODO: API folder

static int sys_open(const char* path, uint32_t flags, mode_t mode)
{
    struct task* task = task_curr();
    list_push_back(&task->fds, vfs_open(vfs_resolve(path))); // TODO: canonicalize the path first
    return task->fds.size - 1;
}

static int sys_close(int fd)
{
    // TODO: remove list element at
}

static ssize_t sys_read(int fdno, void* buf, size_t size)
{
    struct task* task = task_curr();
    //if (fd < 0 || fd >= task->fds.size) return -EBADF;

    struct fd* fd = list_get(&task->fds, fdno);
    ssize_t ret = vfs_read(fd->filp, buf, fd->off, size);
    fd->off += size;
    return ret;
}

static ssize_t sys_write(int fdno, const void* buf, size_t size)
{
    struct task* task = task_curr();
    //if (fd < 0 || fd >= task->fds.size) return -EBADF;

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
    task_execve(task_curr(), path, argv, envp);
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
    // TODO: impl proper
    struct task* task = sched_task_fromid(pid);
    //if (!task) return -ESRCH;
    task_send(task, sig);

    if (task_curr() == task) sched_yield();

    return 0;
}

static int sys_getpid()
{
    return task_curr()->id;
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
    sys_getpid
};

void syscall_handler(struct regs* r)
{
    thread_curr()->syscall_regs = *r;
    syscall_t sc = syscalls[arch_syscall_num(r)];
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