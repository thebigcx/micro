#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("usage: readlink <pathname>\n");
        return -1;
    }

    char link[60];

    if (readlink(argv[1], link, 60) < 0)
    {
        perror("readlink");
        return -1;
    }

    printf("%s\n", link);

    return 0;
}