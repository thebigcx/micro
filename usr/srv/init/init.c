//#include <unistd.h>

extern unsigned long syscall(unsigned long, ...);
extern unsigned long syscall2(unsigned long, ...);

void _start(int argc, char** argv)
{
    syscall(4);
    syscall(3, 1, "Task", 4);
    //syscall2(4);
    for (;;);
}
