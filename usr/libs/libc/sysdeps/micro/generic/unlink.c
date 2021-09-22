#include <unistd.h>
#include <libc/syscall.h>

int unlink(const char* pathname)
{
    return SYSCALL_ERR(unlink, pathname);
}