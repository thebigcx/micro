#include <unistd.h>
#include <stddef.h>
#include <stdint.h>

int main(int argc, char** argv)
{
    if (fork() == 0)
    {
        // Run the shell
        const char* argv[] = { "/usr/bin/term", NULL };
        const char* envp[] = { NULL };
        execve(argv[0], argv, envp);
    }
    else
    {
        // Wait forever
        for(;;);
    }

    return 0;
}
