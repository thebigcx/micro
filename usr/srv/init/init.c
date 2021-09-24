#include <unistd.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/wait.h>

int main(int argc, char** argv)
{
    pid_t pid = fork();
    if (pid == 0)
    {
        // Run the shell
        const char* argv[] = { "/usr/bin/term", NULL };
        const char* envp[] = { NULL };
        execve(argv[0], argv, envp);
    }
    else
    {
        // Wait forever (term won't finish)
        waitpid(pid, NULL, 0);
    }

    return 0;
}
