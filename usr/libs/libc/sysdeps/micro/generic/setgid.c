#include <unistd.h>
#include <libc/syscall.h>

int setgid(gid_t gid)
{
    return SYSCALL_ERR(setgid, gid);
}