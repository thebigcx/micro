#include <unistd.h>
#include <libc/syscall.h>

int setregid(gid_t rgid, gid_t egid)
{
    return SYSCALL_ERR(setregid, rgid, egid);
}