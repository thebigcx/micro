#include <unistd.h>
#include <libc/syscall.h>

gid_t getgid()
{
    return syscall(SYS_getgid);
}