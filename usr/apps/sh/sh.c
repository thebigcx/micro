#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>

#define PATH "/usr/bin/"

void* xmalloc(size_t n)
{
    void* p = malloc(n);
    if (!p) abort();
    return p;
}

int exitcode = 0;

int get_command(char* str, char** bin, char*** argv)
{
    char* saveptr;
    char* token = strtok_r(str, " \0\n", &saveptr);

    printf("command\n");

    if (!token)
        return -1;

    if (token[0] == '/')
    {
        *bin = xmalloc(strlen(token) + 1);
        (*bin)[0] = 0;
    }
    else
    {
        *bin = xmalloc(strlen(token) + strlen(PATH) + 1);
        strcpy(*bin, PATH);
    }
    
    strcpy(*bin + strlen(*bin), token);

    token = strtok_r(NULL, " \0", &saveptr);

    (*argv)[0] = strdup(*bin);
    unsigned int argc = 1;

    while (token)
    {
        (*argv)[argc] = strdup(token);
        argc++;

        token = strtok_r(NULL, " \0", &saveptr);
    }

    (*argv)[argc] = NULL;
    return 0;
}

int main(int argc, char** argv)
{
    exitcode = 0;

    setvbuf(stdout, NULL, _IONBF, 0);

    //seteuid(1000);
    //setegid(1000);

    putenv("HOME=/home/anon");
    chdir(getenv("HOME"));

    while (1)
    {
        char cwd[128];
        getcwd(cwd, 128);

        printf("%d>root@micro:%s$ ", exitcode, cwd);

        /*char* line = xmalloc(256);
        size_t lineptr = 0;
        for (;;)
        {
            char c;
            while (read(STDIN_FILENO, &c, 1) == 0);
            line[lineptr++] = c;
            if (c == '\n') break;
        }
        line[lineptr] = 0;

        if (lineptr == 1)
            continue;*/
        char line[256];
        fgets(line, 256, stdin);

        char* ptr = strchr(line, '\n');
        if (ptr) *ptr = 0;

        char* saveptr;
        char* token = strtok_r(line, "|", &saveptr);

        char* bin;
        char* argv[32];
        char** argv_ptr = argv;
        if (get_command(token, &bin, &argv_ptr))
            continue;

        if (!strncmp(line, "cd", 2))
        {
            if (chdir(argv[1]) != 0)
                perror("cd: ");
            continue;
        }

        if (!strncmp(line, "exit", 4))
            return 0;

        pid_t child = fork();
        if (child == 0)
        {
            const char* envp[] = { NULL };
            
            if (execve(bin, (const char**)argv, envp))
            {
                perror("sh: ");
                continue;
            }
        }
        else
        {
            int status;
            waitpid(-1, &status, 0);

            //if (WIFSIGNALED(status))
            if (!WIFEXITED(status))
            {
                switch (WTERMSIG(status))
                {
                    case SIGSEGV:
                        printf("Segmentation fault\n");
                        break;
                    case SIGFPE:
                        printf("Floating-point exception\n");
                        break;
                    case SIGILL:
                        printf("Illegal instruction\n");
                        break;
                    case SIGABRT:
                        printf("Aborted\n");
                        break;
                    default:
                        printf("Process terminated due to signal %d\n", WTERMSIG(status));
                        break;
                }
            }

            exitcode = WEXITSTATUS(status);
        }
    }
   
    return 0;
}
