#include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("usage: rmdir <directory>\n");
        return -1;
    }

    if (rmdir(argv[1]))
    {
        perror("rmdir");
        return -1;
    }

    return 0;
}