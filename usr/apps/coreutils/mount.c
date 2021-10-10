#include <sys/mount.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    if (argc != 4)
    {
        printf("usage: mount <source> <mountpoint> <fstype>\n");
        return -1;
    }

    if (mount(argv[1], argv[2], argv[3], 0, NULL) == -1)
    {
        perror("mount");
        return -1;
    }

    return 0;
}