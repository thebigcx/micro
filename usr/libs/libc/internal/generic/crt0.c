#include <libc/libc-internal.h>
#include <stdlib.h>
#include <string.h>

extern int main(int, void**, char**);

char** environ;
char* __progname;

// Need to forward-declare
void libc_start_main(int, void**, char**);

void _start(int argc, void** argv, char** envp)
{
    libc_start_main(argc, argv, envp);
}

void libc_start_main(int argc, void** argv, char** envp)
{
    __libc_init();

    // Copy the environment into 'environ'
    environ = malloc(256 * sizeof(char*));
    
    size_t i = 0;
    for (; i < 255 && envp[i]; i++)
        environ[i] = strdup(envp[i]);
    //printf("envp: %x, %x\n", envp, envp[0]);

    environ[i] = NULL;

    environ = envp;
    __progname = strdup(argv[0]);

    int ret = main(argc, argv, envp);

    exit(ret);
}
