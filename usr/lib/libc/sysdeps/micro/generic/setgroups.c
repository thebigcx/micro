#include <grp.h>
#include <libc/syscall.h>

int setgroups(size_t size, const gid_t* list)
{
    return SYSCALL_ERR(setgroups, size, list);
}