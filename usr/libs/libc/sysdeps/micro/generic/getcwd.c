#include <unistd.h>
#include <stdint.h>
#include <libc/syscall.h>

char* getcwd(char* buf, size_t size)
{
    return (char*)(uintptr_t)SYSCALL_ERR(getcwd, buf, size);
}