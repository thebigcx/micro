#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    //printf("%s\n", argv[0]);
    int buffer_size = 0;
    char buffer[100];

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

                    // TODO: make this better (use strtok())
                    char* buf_ptr = buffer;

                    char bin[64];
                    strcpy(bin, "/usr/bin/");
                    char* ptr = bin + 9;

                    char arg1[32];
                    
                    while (*buf_ptr != ' ' && *buf_ptr != 0)
                    {
                        *ptr++ = *buf_ptr++;
                    }
                    *ptr = 0;

                    *buf_ptr++;

                    ptr = arg1;

                    while (*buf_ptr != ' ' && *buf_ptr != 0)
                    {
                        *ptr++ = *buf_ptr++;
                    }
                    *ptr = 0;

                    if (access(bin, F_OK) == -1)
                    {
                        printf("%s: no such file or directory\n", bin);
                        buffer_size = 0;
                        memset(buffer, 0, 100);
                        break;
                    }

                    if (fork() == 0)
                    {
                        const char* argv[] = { "/initrd/cat", arg1, NULL };
                        const char* envp[] = { NULL };

                        //const char* argv[] = { "/initrd/cat", "/initrd/cat", NULL };
                        execve(bin, argv, envp);
                    }

                    memset(buffer, 0, 100);
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
