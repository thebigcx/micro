#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("mkdir: provide a directory name\n");
        return 0;
    }

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