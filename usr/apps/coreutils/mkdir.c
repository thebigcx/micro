#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("usage: mkdir <directory>\n");
        return 0;
    }

    printf("argv[1] == %s\n", argv[1]);
    if (!mkdir(argv[1], 0))
    {
        if (errno == EEXIST)
        {
            printf("mkdir: %s: already exists\n");
            return 0;
        }
        else if (errno == ENOTDIR)
        {
            printf("mkdir: %s: no such file or directory\n");
            return 0;
        }
    }

    return 0;
}