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

int main(int argc, char** argv)
{
    while (1)
    {
        printf("$ ");

        char* line = malloc(256);
        fgets(line, 256, stdin);

        char* ptr = strchr(line, '\n');
        if (ptr) *ptr = 0;

        char* saveptr;
        char* token = strtok_r(line, " \0\n", &saveptr);

        if (!token)
            continue;

        char* bin;
        if (token[0] == '/')
        {
            bin = malloc(strlen(token) + 1);
            bin[0] = 0;
        }
        else
        {
            bin = malloc(strlen(token) + strlen(PATH) + 1);
            strcpy(bin, PATH);
        }
        
        strcpy(bin + strlen(bin), token);

        token = strtok_r(NULL, " \0", &saveptr);

        char* argv[32];
        argv[0] = strdup(bin);
        unsigned int argc = 1;

        while (token)
        {
            argv[argc] = strdup(token);
            argc++;

            token = strtok_r(NULL, " \0", &saveptr);
        }

        argv[argc] = NULL;

        if (!strncmp(line, "cd", 2))
        {
            if (chdir(argv[1]) != 0)
            {
                printf("cd: %s: no such file or directory\n", argv[1]);
            }
            continue;
        }

        if (access(bin, F_OK) == -1)
        {
            printf("%s: no such file or directory\n", bin);
            continue;
        }

        pid_t child = fork();
        if (child == 0)
        {
            const char* envp[] = { NULL };
            execve(bin, (const char**)argv, envp);
        }
        else
        {
            int status;
            waitpid(child, &status, 0);

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
        }
    }
   
    return 0;
}
