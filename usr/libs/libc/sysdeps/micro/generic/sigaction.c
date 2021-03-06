#include <signal.h>
#include <libc/syscall.h>

static void __libc_sig_restorer()
{
    // syscall(SYS_sigreturn)
    asm (
        "mov $49, %rax\n"
        "int $0x80\n"
    );
}

int sigaction(int signum, const struct sigaction* act, struct sigaction* old)
{
    if (act && act->sa_handler > SIG_IGN)
    {
        struct sigaction ksigact = *act;
        ksigact.sa_restorer = __libc_sig_restorer;

        return SYSCALL_ERR(sigaction, signum, &ksigact, old);
    }
    else
        return SYSCALL_ERR(sigaction, signum, act, old);
}