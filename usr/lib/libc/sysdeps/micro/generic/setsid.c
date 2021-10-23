#include <unistd.h>
#include <libc/syscall.h>

pid_t setsid()
{
    return SYSCALL_ERR(setsid);
}