#include <sys/stat.h>
#include <libc/syscall.h>

int chown(const char* pathname, uid_t uid, gid_t gid)
{
    return SYSCALL_ERR(chown, pathname, uid, gid);
}