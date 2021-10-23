#include <sys/stat.h>
#include <libc/syscall.h>

int chmod(const char* pathname, mode_t mode)
{
    return SYSCALL_ERR(chmod, pathname, mode);
}