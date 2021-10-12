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
#include <micro/syscall.h>
#include <micro/debug.h>

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
    struct task* task = pid ? sched_task_fromid(pid) : task_curr();
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

// TODO: use the suid field in task
SYSCALL_DEFINE(setreuid, uid_t ruid, uid_t euid)
{
    struct task* task = task_curr();

    if (ruid != (uid_t)-1)
    {
        if (task->euid) return -EPERM;
        task->ruid = ruid;
    }

    if (euid != (uid_t)-1)
    {
        if (task->euid) return -EPERM;
        task->euid = euid;
    }

    return 0;
}

SYSCALL_DEFINE(setregid, gid_t rgid, gid_t egid)
{
    struct task* task = task_curr();

    if (rgid != (gid_t)-1)
    {
        if (task->egid) return -EPERM;
        task->rgid = rgid;
    }

    if (egid != (gid_t)-1)
    {
        if (task->egid) return -EPERM;
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

SYSCALL_DEFINE(prctl, int option, unsigned long arg2, unsigned long arg3,
               unsigned long arg4, unsigned long arg5)
{
    return 0;
}

SYSCALL_DEFINE(gettid)
{
    return task_curr()->pid;
}

SYSCALL_DEFINE(set_tid_address, int* tidptr)
{
    *tidptr = 0;
    return task_curr()->pid;
}

SYSCALL_DEFINE(exit_group, int status)
{
	return sys_exit(status);
}

typedef uintptr_t (*syscall_t)();

// System call table
static void* syscalls[] =
{
    [SYS_open]            = &sys_open,
    [SYS_close]           = &sys_close,
    [SYS_read]            = &sys_read,
    [SYS_write]           = &sys_write,
    [SYS_fork]            = &sys_fork,
    [SYS_execve]          = &sys_execve,
    [SYS_exit]            = &sys_exit,
    [SYS_kill]            = &sys_kill,
    [SYS_getpid]          = &sys_getpid,
    [SYS_access]          = &sys_access,
    [SYS_lseek]           = &sys_lseek,
    [SYS_mmap]            = &sys_mmap,
    [SYS_munmap]          = &sys_munmap,
    [SYS_chdir]           = &sys_chdir,
    [SYS_getcwd]          = &sys_getcwd,
    [SYS_getdents64]      = &sys_getdents,
    [SYS_mkdir]           = &sys_mkdir,
    [SYS_ioctl]           = &sys_ioctl,
    [SYS_time]            = &sys_time,
    [SYS_dup]             = &sys_dup,
    [SYS_dup2]            = &sys_dup2,
    [SYS_init_module]     = &sys_insmod,
    [SYS_delete_module]   = &sys_rmmod,
    [SYS_mount]           = &sys_mount,
    [SYS_umount2]         = &sys_umount,
    [SYS_pread64]         = &sys_pread,
    [SYS_pwrite64]        = &sys_pwrite,
    [SYS_gettimeofday]    = &sys_gettimeofday,
    [SYS_ptrace]          = &sys_ptrace,
    [SYS_stat]            = &sys_stat,
    [SYS_fstat]           = &sys_fstat,
    [SYS_lstat]           = &sys_lstat,
    [SYS_unlink]          = &sys_unlink,
    [SYS_chmod]           = &sys_chmod,
    [SYS_setreuid]        = &sys_setreuid,
    [SYS_chown]           = &sys_chown,
    [SYS_readlink]        = &sys_readlink,
    [SYS_getuid]          = &sys_getuid,
    [SYS_geteuid]         = &sys_geteuid,
    [SYS_getgid]          = &sys_getgid,
    [SYS_getegid]         = &sys_getegid,
    [SYS_getgroups]       = &sys_getgroups,
    [SYS_setgroups]       = &sys_setgroups,
    [SYS_setregid]        = &sys_setregid,
    [SYS_symlink]         = &sys_symlink,
    [SYS_link]            = &sys_link,
    [SYS_rt_sigaction]    = &sys_sigaction,
    [SYS_rt_sigreturn]    = &sys_sigreturn,
    [SYS_rt_sigprocmask]  = &sys_sigprocmask,
    [SYS_umask]           = &sys_umask,
    [SYS_pipe]            = &sys_pipe,
    [SYS_rename]          = &sys_rename,
    [SYS_rmdir]           = &sys_rmdir,
    [SYS_reboot]          = &sys_reboot,
    [SYS_uname]           = &sys_uname,
    [SYS_getppid]         = &sys_getppid,
    [SYS_fchmod]          = &sys_fchmod,
    [SYS_mknod]           = &sys_mknod,
    [SYS_setuid]          = &sys_setuid,
    [SYS_setgid]          = &sys_setgid,
    [SYS_fcntl]           = &sys_fcntl,
    [SYS_utime]           = &sys_utime,
    [SYS_utimes]          = &sys_utimes,
    [SYS_getpgid]         = &sys_getpgid,
    [SYS_setpgid]         = &sys_setpgid,
    [SYS_getsid]          = &sys_getsid,
    [SYS_setsid]          = &sys_setsid,
    [SYS_arch_prctl]      = &sys_prctl,
    [SYS_wait4]           = &sys_wait4,
    [SYS_gettid]          = &sys_gettid,
    [SYS_readv]           = &sys_readv,
    [SYS_writev]          = &sys_writev,
    [SYS_tkill]           = &sys_tkill,
    [SYS_brk]             = &sys_brk,
    [SYS_mprotect]        = &sys_mprotect,
    [SYS_set_tid_address] = &sys_set_tid_address,
    [SYS_exit_group]      = &sys_exit_group,
    [SYS_socket]          = &sys_socket,
    [SYS_bind]            = &sys_bind,
    [SYS_accept]          = &sys_accept,
    [SYS_fchown]          = &sys_fchown,
    [SYS_lchown]          = &sys_lchown
};

void syscall_handler(struct regs* r)
{
    uintptr_t n = arch_syscall_num(r);
    if (n >= sizeof(syscalls) / sizeof(syscall_t) || !syscalls[n])
    {
        printk("nosys: %d\n", n);
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
    idt_set_handler(0x80, syscall_handler);
}
