#include <sys/stat.h>
#include <libc/syscall.h>

int lstat(const char* path, struct stat* buf)
{
    return SYSCALL_ERR(lstat, path, buf);
}