#include <fcntl.h>
#include <libc/syscall.h>

int open(const char* filename, int flags, mode_t mode)
{
    return SYSCALL_ERR(open, filename, flags, mode);
}