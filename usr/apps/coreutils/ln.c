#include <unistd.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        printf("usage: ln [-s] <target> <linkpath>\n");
        return -1;
    }

    // TODO: hardlinks and command line options
    //if (symlink(argv[1], argv[2]))
    if (link(argv[1], argv[2]))
    {
        perror("ln: ");
        return -1;
    }

    return 0;
}