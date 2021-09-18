#include <stdlib.h>
#include <sys/syscall.h>

void exit(int status)
{
    syscall(SYS_exit, status);
}