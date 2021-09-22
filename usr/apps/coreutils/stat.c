#include <sys/stat.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("usage: stat <filename>\n");
        return -1;
    }

    struct stat buf;
    if (stat(argv[1], &buf))
    {
        perror("stat: ");
        return -1;
    }

    printf("Inode: %ld\n", buf.st_ino);
    printf("User ID: %ld\n", buf.st_uid);
    printf("Group ID: %ld\n", buf.st_gid);
    printf("Size: %ld\n", buf.st_size);

    return 0;
}