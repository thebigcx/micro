#include <libc/libc-internal.h>
#include <stdlib.h>

extern int main(int, void**);

char** environ;

// Need to forward-declare
void libc_start_main(int (*)(), int, void**, char**);

void _start(int argc, void** argv, char** envp)
{
    libc_start_main(main, argc, argv, envp);
}

void libc_start_main(int (*main)(), int argc, void** argv, char** envp)
{
    __libc_init();

    environ = envp;

    int ret = main(argc, argv);

    exit(ret);
}
