#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <string.h>

void dump_regs(struct user_regs_struct* regs)
{
    printf("rax=%x\n", regs->rax);
    printf("rbx=%x\n", regs->rbx);
    printf("rcx=%x\n", regs->rcx);
    printf("rdx=%x\n", regs->rdx);
    printf("rdi=%x\n", regs->rdi);
    printf("rsi=%x\n", regs->rsi);
    printf("rbp=%x\n", regs->rbp);
    printf("rsp=%x\n", regs->rsp);
    printf("rip=%x\n", regs->rip);
    printf("cs=%x\n", regs->cs);
    printf("ss=%x\n", regs->ss);
    printf("ds=%x\n", regs->ds);
    printf("es=%x\n", regs->es);
    printf("fs=%x\n", regs->fs);
    printf("rflags=%x\n", regs->eflags);
    printf("fs_base=%x\n", regs->fs_base);
    printf("gs_base=%x\n", regs->gs_base);
    printf("r8=%x\n", regs->r8);
    printf("r9=%x\n", regs->r9);
    printf("r10=%x\n", regs->r10);
    printf("r11=%x\n", regs->r11);
    printf("r12=%x\n", regs->r12);
    printf("r13=%x\n", regs->r13);
    printf("r14=%x\n", regs->r14);
    printf("r15=%x\n", regs->r15);
}

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

        const char** args = &argv[1];
        if (execv(args[0], args) == -1)
        {
            perror("db");
            return -1;
        }
    }
    
    printf("tracing %s...\n", argv[1]);

    waitpid(pid, NULL, 0); // Wait for SIGSTOP
    
    ptrace(PTRACE_CONT, pid, NULL, NULL);

    for (;;)
    {
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status))
        {
            printf("process exited normally (exit code %d)\n", WEXITSTATUS(status));
            break;
        }
        else
        {
            printf("Process received signal %d, %s\n", WSTOPSIG(status), strsignal(WSTOPSIG(status)));

            struct user_regs_struct regs;
            ptrace(PTRACE_GETREGS, pid, NULL, &regs);

            dump_regs(&regs);
            break;
        }
    }
    
    return 0;
}