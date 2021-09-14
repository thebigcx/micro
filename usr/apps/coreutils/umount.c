#include <sys/mount.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("usage: umount <mountpoint>\n");
        return -1;
    }

    if (umount(argv[1]) == -1)
    {
        perror("umount: ");
        return -1;
    }

    return 0;
}