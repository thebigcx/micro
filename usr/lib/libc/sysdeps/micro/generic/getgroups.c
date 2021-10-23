#include <unistd.h>
#include <libc/syscall.h>

int getgroups(int size, gid_t list[])
{
    return SYSCALL_ERR(getgroups, size, list);
}