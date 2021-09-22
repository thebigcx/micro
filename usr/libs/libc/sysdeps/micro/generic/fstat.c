#include <sys/stat.h>
#include <libc/syscall.h>

int fstat(int fd, struct stat* buf)
{
    return SYSCALL_ERR(fstat, fd, buf);
}