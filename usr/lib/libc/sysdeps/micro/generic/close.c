#include <fcntl.h>
#include <libc/syscall.h>

int close(int fd)
{
    return SYSCALL_ERR(close, fd);
}