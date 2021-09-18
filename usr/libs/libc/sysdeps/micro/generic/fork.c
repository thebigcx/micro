#include <unistd.h>
#include <libc/syscall.h>

pid_t fork()
{
    return SYSCALL_ERR(fork);
}