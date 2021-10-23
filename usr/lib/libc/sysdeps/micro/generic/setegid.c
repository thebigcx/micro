#include <unistd.h>
#include <libc/syscall.h>

int setegid(gid_t gid)
{
    return setregid(-1, gid);
}