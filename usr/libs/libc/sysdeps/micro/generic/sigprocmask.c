#include <signal.h>
#include <libc/syscall.h>

int sigprocmask(int how, const sigset_t* set, sigset_t* oldset)
{
    return SYSCALL_ERR(sigprocmask, how, set, oldset);
}