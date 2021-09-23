#include <sys/stat.h>
#include <stdlib.h>
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

    printf("%s\n", realpath(argv[1], NULL));

    printf("Inode: %ld\n", buf.st_ino);
    printf("User ID: %ld\n", buf.st_uid);
    printf("Group ID: %ld\n", buf.st_gid);
    printf("Size: %ld\n", buf.st_size);
    printf("Link count: %d\n", buf.st_nlink);

    printf("Last access: %s", ctime(&buf.st_atime));
    printf("Creation time: %s", ctime(&buf.st_ctime));
    printf("Last modification: %s", ctime(&buf.st_mtime));

    printf("Type: ");

    if      (S_ISREG(buf.st_mode)) printf("file\n");
    else if (S_ISDIR(buf.st_mode)) printf("directory\n");
    else if (S_ISBLK(buf.st_mode)) printf("block device\n");
    else if (S_ISCHR(buf.st_mode)) printf("character device\n");
    else if (S_ISFIFO(buf.st_mode)) printf("FIFO\n");
    else if (S_ISLNK(buf.st_mode)) printf("symbolic link\n");
    else if (S_ISSOCK(buf.st_mode)) printf("socket\n");
    else printf("unknown\n");

    return 0;
}