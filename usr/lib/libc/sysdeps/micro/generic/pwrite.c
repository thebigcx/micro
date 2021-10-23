#include <unistd.h>
#include <libc/syscall.h>

ssize_t pwrite(int fd, const void* buf, size_t count, off_t off)
{
    return SYSCALL_ERR(pwrite, fd, buf, count, off);
}