#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>

void _start(int argc, char** argv)
{
    if (fork() == 0)
    {
        execve("/initrd/init2", NULL, NULL);
    }
    
    write(1, "Parent", 6);
    for (;;);
}
