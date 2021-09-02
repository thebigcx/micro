#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>

void _start(int argc, char** argv)
{
    write(1, "init2", 5);
    for(;;);
}
