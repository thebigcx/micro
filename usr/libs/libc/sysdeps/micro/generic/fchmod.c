#include <sys/stat.h>
#include <libc/syscall.h>

int fchmod(int fd, mode_t mode)
{
    return SYSCALL_ERR(fchmod, fd, mode);
}