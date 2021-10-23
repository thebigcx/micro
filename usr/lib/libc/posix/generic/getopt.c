#include <unistd.h>
#include <string.h>

char* optarg;
int optind = 1, opterr, optopt;

int getopt(int argc, char* const argv[], const char* optstr)
{
    if (optind == argc)
        return -1;

    while (argv[optind][0] != '-')
    {
        if (++optind >= argc)
            return -1;
    }

    char* sub = strchr(optstr, argv[optind][1]);

    if (!sub)
    {
        printf("Unrecognised option '-%c'\n", argv[optind][1]);
        return '?';
    }

    char o = argv[optind][1];

    if (sub[1] == ':')
    {
        if (!argv[optind][2])
        {
            printf("Expected argument for option '%c'\n", argv[optind][1]);
            return ':';
        }

        optarg = &argv[optind][2];
    }

    optind++;
    
    return o;
}