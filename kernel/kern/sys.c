#include <micro/sys.h>
#include <micro/types.h>
#include <micro/vfs.h>
#include <micro/thread.h>
#include <arch/reg.h>
#include <arch/cpu.h>
#include <micro/errno.h>

SYSCALL_DEFINE(getpid)
{
    return task_curr()->id;
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
    &sys_unlink
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