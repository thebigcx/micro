#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("usage: mkdir <directory>\n");
        return -1;
    }

    printf("argv[1] == %s\n", argv[1]);
    if (mkdir(argv[1], 0) == -1)
    {
        perror("mkdir: ");
        return -1;
    }

    return 0;
}