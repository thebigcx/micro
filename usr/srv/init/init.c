#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>

void _start(int argc, char** argv)
{
    char code[100];
    int fd = open("/initrd/init", 0, 0);
    read(fd, code, 100);

    write(1, code, 100);
    exit(0);


    if (fork() == 0)
    {
        //kill(0, 0);
        //*((unsigned int*)0x10000000000000) = 1000;
        int i = 100 / 0;
        //raise(0);
        execve("/initrd/init2", NULL, NULL);
    }
    
    write(1, "Parent", 6);
    exit(0);
    for (;;);
}
