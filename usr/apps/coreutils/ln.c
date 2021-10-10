#include <unistd.h>
#include <stdio.h>

int soft = 0;

void usage()
{
    printf("usage: ln [-s] <target> <linkpath>\n");
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        usage();
        return -1;
    }

    char opt;
    while ((opt = getopt(argc, argv, "s")) != -1)
    {
        switch (opt)
        {
            case 's':
                soft = 1;
                break;
            default:
                return -1;
        }
    }

    size_t i = 1;
    while (i < argc && argv[i][0] == '-') i++;

    if (i == argc)
    {
        usage();
        return -1;
    }

    // TODO: hardlinks and command line options

    int res;
    if (soft)
        res = symlink(argv[i], argv[i + 1]);
    else
        res = link(argv[i], argv[i + 1]);

    if (res)
    {
        perror("ln");
        return -1;
    }

    return 0;
}