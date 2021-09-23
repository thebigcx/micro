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
        char cwd[128];
        getcwd(cwd, 128);

        printf("root@micro:%s$ ", cwd);

        char* line = malloc(256);
        size_t lineptr = 0;
        for (;;)
        {
            char c;
            while (read(STDIN_FILENO, &c, 1) == 0);
            line[lineptr++] = c;
            if (c == '\n') break;
        }

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
                perror("cd: ");
            continue;
        }

        if (!strncmp(line, "exit", 4))
            return 0;

        if (access(bin, F_OK) == -1)
        {
            perror("");
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
