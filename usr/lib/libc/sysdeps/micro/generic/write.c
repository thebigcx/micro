#include <unistd.h>
#include <libc/syscall.h>

ssize_t write(int fd, const void* buf, size_t count)
{
    return SYSCALL_ERR(write, fd, buf, count);
}