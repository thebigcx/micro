#include <unistd.h>
#include <libc/syscall.h>

char* getcwd(char* buf, size_t size)
{
    return SYSCALL_ERR(getcwd, buf, size);
}