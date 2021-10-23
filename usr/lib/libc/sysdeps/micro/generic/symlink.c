#include <unistd.h>
#include <libc/syscall.h>

int symlink(const char* target, const char* linkpath)
{
    return SYSCALL_ERR(symlink, target, linkpath);
}