#include <unistd.h>
#include <libc/syscall.h>

pid_t getsid(pid_t pid)
{
    return SYSCALL_ERR(getsid, pid);
}