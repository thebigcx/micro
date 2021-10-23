#include <sys/stat.h>
#include <libc/syscall.h>

int stat(const char* path, struct stat* buf)
{
    return SYSCALL_ERR(stat, path, buf);
}