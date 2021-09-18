#include <unistd.h>
#include <libc/syscall.h>

extern char** environ;

int execv(const char* path, const char* argv[])
{
    return execve(path, argv, (const char**)environ); 
}