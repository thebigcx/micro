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
    int buffer_size = 0;
    char buffer[128];
    
    while (1)
    {
        printf("$ ");

        while (1)
        {
            char c;
            if (read(0, &c, 1))
            {
                printf("%c", c);

                if (c == '\n')
                {
                    buffer[buffer_size] = 0;

                    char* saveptr;
                    char* token = strtok_r(buffer, " \0", &saveptr);

                    if (!token)
                    {
                        buffer_size = 0;
                        break;
                    }

                    char* bin = malloc(strlen(token) + strlen(PATH) + 1);
                    strcpy(bin, PATH);
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

                    if (!strncmp(buffer, "cd", 2))
                    {
                        if (chdir(argv[1]) != 0)
                        {
                            printf("cd: %s: no such file or directory\n", argv[1]);
                        }
                        buffer_size = 0;
                        break;
                    }

                    if (access(bin, F_OK) == -1)
                    {
                        printf("%s: no such file or directory\n", bin);
                        buffer_size = 0;
                        break;
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

                        if (WIFEXITED(status))
                            printf("Process exited normally (status %d)\n", WEXITSTATUS(status));
                        else// if (WIFSIGNALED(status))
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

                    buffer_size = 0;
                    break;
                }
                else if (c == '\b')
                {
                    buffer_size--;
                }
                else
                {
                    buffer[buffer_size++] = c;
                }
            }
        }
    }
   
    return 0;
}
