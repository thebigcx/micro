#include <unistd.h>
#include <libc/syscall.h>

ssize_t pread(int fd, void* buf, size_t count, off_t off)
{
    return SYSCALL_ERR(pread, fd, buf, count, off);
}