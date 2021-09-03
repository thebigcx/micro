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

                    char bin[32];
                    char* ptr = bin;

                    char arg1[32];
                    
                    while (*buf_ptr != ' ' && *buf_ptr != 0)
                    {
                        *ptr++ = *buf_ptr++;
                    }

                    *buf_ptr++;

                    ptr = arg1;

                    while (*buf_ptr != ' ' && *buf_ptr != 0)
                    {
                        *ptr++ = *buf_ptr++;
                    }

                    if (access(bin, F_OK) == -1)
                    {
                        printf("%s: no such file or directory\n", bin);
                        buffer_size = 0;
                        break;
                    }

                    if (fork() == 0)
                    {
                        printf("executing %s with arg %s\n", bin, arg1);
                        const char* argv[] = { "/initrd/cat", arg1, NULL };

                        //const char* argv[] = { "/initrd/cat", "/initrd/cat", NULL };
                        execve(bin, argv, NULL);
                    }

                    buffer_size = 0;
                    break;
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
