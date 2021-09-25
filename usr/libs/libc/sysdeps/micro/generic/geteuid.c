#include <unistd.h>
#include <libc/syscall.h>

uid_t geteuid()
{
    return syscall(SYS_geteuid);
}