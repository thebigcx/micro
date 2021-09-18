#include <signal.h>
#include <libc/syscall.h>

int kill(pid_t pid, int sig)
{
    return SYSCALL_ERR(kill, pid, sig);
}