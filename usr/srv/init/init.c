#include <unistd.h>

extern unsigned long syscall(unsigned long, ...);
extern unsigned long syscall2(unsigned long, ...);

void _start(int argc, char** argv)
{
    if (syscall(4) == 0)
    {
        syscall(3, 1, "Child", 5);
    }
    else
    {
        exit(1);
        syscall(3, 1, "Parent", 6);
    }
    //syscall2(4);
    for (;;);
}
