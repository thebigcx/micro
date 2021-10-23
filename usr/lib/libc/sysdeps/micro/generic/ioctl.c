#include <unistd.h>
#include <libc/syscall.h>

int ioctl(int fd, unsigned long request, void* argp)
{
    return SYSCALL_ERR(ioctl, fd, request, argp);
}