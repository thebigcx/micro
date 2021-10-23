#include <unistd.h>
#include <libc/syscall.h>

int link(const char* old, const char* new)
{
    return SYSCALL_ERR(link, old, new);
}