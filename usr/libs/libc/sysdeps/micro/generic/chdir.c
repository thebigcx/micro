#include <unistd.h>
#include <libc/syscall.h>

int chdir(const char* path)
{
    return SYSCALL_ERR(chdir, path);
}