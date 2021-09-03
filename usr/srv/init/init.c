#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
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
                    if (access(buffer, F_OK) == -1)
                    {
                        printf("%s: no such file or directory\n", buffer);
                        break;
                    }

                    if (fork() == 0)
                    {
                        execve(buffer, NULL, NULL);
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
