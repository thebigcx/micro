#include <sys/stat.h>
#include <libc/syscall.h>

mode_t umask(mode_t mask)
{
    return SYSCALL_ERR(umask, mask);
}