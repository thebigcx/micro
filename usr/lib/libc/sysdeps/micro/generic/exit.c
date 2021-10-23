#include <stdlib.h>
#include <stdio.h>
#include <sys/syscall.h>

void exit(int status)
{
    fflush(stdout);
    fflush(stderr);

    syscall(SYS_exit, status);
}