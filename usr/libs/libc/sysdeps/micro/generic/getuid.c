#include <unistd.h>
#include <libc/syscall.h>

uid_t getuid()
{
    return syscall(SYS_getuid);
}