#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>

void _start(int argc, char** argv)
{
    execve("/initrd/init", NULL, NULL);
    
    if (fork() == 0)
    {
        //syscall(3, 1, "Child", 5);
        write(1, "Child", 5);
    }
    else
    {
        exit(1);
        write(1, "Parent", 6);
    }
    //syscall2(4);
    for (;;);
}
