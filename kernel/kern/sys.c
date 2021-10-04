#include <micro/sys.h>
#include <micro/types.h>
#include <micro/vfs.h>
#include <micro/thread.h>
#include <arch/reg.h>
#include <arch/cpu.h>
#include <micro/errno.h>
#include <micro/heap.h>
#include <micro/utsname.h>
#include <micro/stdlib.h>
#include <micro/sched.h>

SYSCALL_DEFINE(getpid)
{
    return task_curr()->pid;
}

SYSCALL_DEFINE(getuid)
{
    return task_curr()->ruid;
}

SYSCALL_DEFINE(geteuid)
{
    return task_curr()->euid;
}

SYSCALL_DEFINE(getgid)
{
    return task_curr()->rgid;
}

SYSCALL_DEFINE(getegid)
{
    return task_curr()->egid;
}

SYSCALL_DEFINE(getppid)
{
    return task_curr()->parent->pid;
}

SYSCALL_DEFINE(getpgid, pid_t pid)
{
    struct task* task = sched_task_fromid(pid);
    if (!task) return -ESRCH;
    return task->pgid;
}

// TODO: more error checking
SYSCALL_DEFINE(setpgid, pid_t pid, pid_t pgid)
{
    if (pgid < 0) return -EINVAL;

    if (!pid) pid = task_curr()->pid;

    struct task* task = sched_task_fromid(pid);
    if (!task || (task != task_curr() && task->parent != task_curr()))
        return -ESRCH;
    
    // Session leader
    if (task->sid == task->pid)
        return -EPERM;

    if (!pgid) pgid = pid;

    task->pgid = pgid;
    return 0;
}

SYSCALL_DEFINE(getsid, pid_t pid)
{
    struct task* task = sched_task_fromid(pid);
    if (!task) return -ESRCH;
    return task->sid;
}

SYSCALL_DEFINE(setsid)
{
    struct task* task = task_curr();

    // Already a process group leader
    if (task->pgid == task->pid)
        return -EPERM;

    task->sid  = task->pid;
    task->pgid = task->pid;

    return task->sid;
}

SYSCALL_DEFINE(setreuid, uid_t ruid, uid_t euid)
{
    struct task* task = task_curr();

    if (ruid != -1)
    {
        if (task->euid != 0) return -EPERM;
        task->ruid = ruid;
    }

    if (euid != -1)
    {
        if (task->euid != 0) return -EPERM;
        task->euid = euid;
    }

    return 0;
}

SYSCALL_DEFINE(setregid, gid_t rgid, gid_t egid)
{
    struct task* task = task_curr();

    if (rgid != -1)
    {
        if (task->egid != 0) return -EPERM;
        task->rgid = rgid;
    }

    if (egid != -1)
    {
        if (task->egid != 0) return -EPERM;
        task->egid = egid;
    }

    return 0;
}

SYSCALL_DEFINE(setuid, uid_t uid)
{
    // TODO: proper implementation
    sys_setreuid(-1, uid);
    return 0;
}

SYSCALL_DEFINE(setgid, gid_t gid)
{
    // TODO: proper implementation
    sys_setregid(-1, gid);
    return 0;
}

SYSCALL_DEFINE(getgroups, int size, gid_t list[])
{
    PTRVALID(list);
    if (size < 0) return -EINVAL;

    struct task* task = task_curr();
    if (task->groupcnt > (size_t)size) return -EINVAL;

    size = task->groupcnt;

    for (int i = 0; i < size; i++)
        list[i] = task->groups[i];

    return size;
}

// TODO: privileges
SYSCALL_DEFINE(setgroups, size_t size, const gid_t* list)
{
    struct task* task = task_curr();

    kfree(task->groups);
    task->groupcnt = size;

    if (!list) return 0;

    task->groups = kmalloc(size * sizeof(gid_t));
    PTRVALID(list);
    
    for (size_t i = 0; i < size; i++)
    {
        PTRVALID(list + i);
        task->groups[i] = list[i];        
    }

    return 0;
}

SYSCALL_DEFINE(umask, mode_t umask)
{
    struct task* task = task_curr();

    mode_t old = task->umask;
    task->umask = umask;
    return old;
}

SYSCALL_DEFINE(uname, struct utsname* buf)
{
    strcpy(buf->sysname, "Micro");
    strcpy(buf->release, "0.0.1");
    strcpy(buf->version, "INITIAL VERSION");
    strcpy(buf->machine, "x86_64");
    strcpy(buf->nodename, "micro");
    return 0;
}

typedef uintptr_t (*syscall_t)();

static void* syscalls[] =
{
    &sys_open,
    &sys_close,
    &sys_read,
    &sys_write,
    &sys_fork,
    &sys_execve,
    &sys_exit,
    &sys_kill,
    &sys_getpid,
    &sys_access,
    &sys_lseek,
    &sys_waitpid,
    &sys_mmap,
    &sys_munmap,
    &sys_chdir,
    &sys_getcwd,
    &sys_getdents,
    &sys_mkdir,
    &sys_ioctl,
    &sys_time,
    &sys_dup,
    &sys_dup2,
    &sys_insmod,
    &sys_rmmod,
    &sys_mount,
    &sys_umount,
    &sys_pread,
    &sys_pwrite,
    &sys_ptsname,
    &sys_gettimeofday,
    &sys_ptrace,
    &sys_stat,
    &sys_fstat,
    &sys_lstat,
    &sys_unlink,
    &sys_chmod,
    &sys_setreuid,
    &sys_chown,
    &sys_readlink,
    &sys_getuid,
    &sys_geteuid,
    &sys_getgid,
    &sys_getegid,
    &sys_getgroups,
    &sys_setgroups,
    &sys_setregid,
    &sys_symlink,
    &sys_link,
    &sys_sigaction,
    &sys_sigreturn,
    &sys_sigprocmask,
    &sys_umask,
    &sys_pipe,
    &sys_rename,
    &sys_rmdir,
    &sys_reboot,
    &sys_uname,
    &sys_getppid,
    &sys_fchmod,
    &sys_mknod,
    &sys_setuid,
    &sys_setgid,
    &sys_fcntl,
    &sys_utime,
    &sys_utimes,
    &sys_getpgid,
    &sys_setpgid,
    &sys_getsid,
    &sys_setsid
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