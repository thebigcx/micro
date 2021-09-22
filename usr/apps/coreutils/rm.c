#include <unistd.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("usage: rm <filename>\n");
        return -1;
    }

    if (unlink(argv[1]))
    {
        perror("rm: ");
        return -1;
    }

    return 0;
}