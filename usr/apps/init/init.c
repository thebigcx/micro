#include <unistd.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/wait.h>
#include <sys/syscall.h>

int main(int argc, char** argv)
{
    pid_t pid = fork();
    if (pid == 0)
    {
        // Run the shell
        char* const argv[] = { "/usr/bin/term", NULL };
        char* const envp[] = { NULL };
        execve(argv[0], argv, envp);
    }
    else
    {
        waitpid(pid, NULL, 0);
    }

    syscall(SYS_reboot);

    return 0;
}
