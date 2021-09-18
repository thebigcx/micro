#include <unistd.h>
#include <libc/syscall.h>

ssize_t read(int fd, void* buf, size_t count)
{
    return SYSCALL_ERR(read, fd, buf, count);
}