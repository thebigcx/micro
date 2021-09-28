#include <unistd.h>
#include <libc/syscall.h>

int pipe(int fds[2])
{
    return SYSCALL_ERR(pipe, fds);
}