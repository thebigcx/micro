#include <unistd.h>
#include <libc/syscall.h>

ssize_t readlink(const char* path, char* buf, size_t n)
{
    return SYSCALL_ERR(readlink, path, buf, n);
}