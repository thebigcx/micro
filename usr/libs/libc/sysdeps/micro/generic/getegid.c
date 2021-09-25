#include <unistd.h>
#include <libc/syscall.h>

gid_t getegid()
{
    return syscall(SYS_getegid);
}