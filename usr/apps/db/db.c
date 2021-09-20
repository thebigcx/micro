#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ptrace.h>
#include <sys/user.h>

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("usage: db <executable>\n");
        return -1;
    }

    pid_t pid = fork();
    if (pid == 0)
    {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        raise(SIGSTOP);

        const char* args[] = { argv[1], NULL };
        if (execv(args[0], args) == -1)
        {
            perror("db: ");
            return -1;
        }
    }
    
    printf("tracing %s...\n", argv[1]);

    struct user_regs r;
    ptrace(PTRACE_GETREGS, pid, NULL, &r);

    printf("%x\n", r.rip);
    
    for (;;);
    
    return 0;
}