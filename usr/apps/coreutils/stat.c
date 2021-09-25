#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("usage: stat <filename>\n");
        return -1;
    }

    struct stat buf;
    if (lstat(argv[1], &buf))
    {
        perror("stat: ");
        return -1;
    }

    printf("%s", realpath(argv[1], NULL));

    // Cool arrow thing if symlink
    if (S_ISLNK(buf.st_mode))
    {
        char link[60];
        readlink(argv[1], link, 60);
        printf(" -> %s\n", link);
    }
    else
        printf("\n");

    printf("Inode: %ld\n", buf.st_ino);
    printf("User ID: %ld\n", buf.st_uid);
    printf("Group ID: %ld\n", buf.st_gid);
    printf("Size: %ld\n", buf.st_size);
    printf("Link count: %d\n", buf.st_nlink);

    printf("Last access: %s", ctime(&buf.st_atime));
    printf("Creation time: %s", ctime(&buf.st_ctime));
    printf("Last modification: %s", ctime(&buf.st_mtime));

    printf("Type: ");

    if      (S_ISREG(buf.st_mode))  printf("file\n");
    else if (S_ISDIR(buf.st_mode))  printf("directory\n");
    else if (S_ISBLK(buf.st_mode))  printf("block device\n");
    else if (S_ISCHR(buf.st_mode))  printf("character device\n");
    else if (S_ISFIFO(buf.st_mode)) printf("FIFO\n");
    else if (S_ISLNK(buf.st_mode))  printf("symbolic link\n");
    else if (S_ISSOCK(buf.st_mode)) printf("socket\n");
    else printf("unknown\n");

    printf("Access: ");

    if (buf.st_mode & S_IRUSR) printf("r");
    else printf("-");
    if (buf.st_mode & S_IWUSR) printf("w");
    else printf("-");
    if (buf.st_mode & S_IXUSR) printf("x");
    else if (buf.st_mode & S_ISUID) printf("s");
    else printf("-");
    

    if (buf.st_mode & S_IRGRP) printf("r");
    else printf("-");
    if (buf.st_mode & S_IWGRP) printf("w");
    else printf("-");
    if (buf.st_mode & S_IXGRP) printf("x");
    else if (buf.st_mode & S_ISGID) printf("s");
    else printf("-");

    if (buf.st_mode & S_IROTH) printf("r");
    else printf("-");
    if (buf.st_mode & S_IWOTH) printf("w");
    else printf("-");
    if (buf.st_mode & S_IXOTH) printf("x");
    else if (buf.st_mode & S_ISUID) printf("t");
    else printf("-");

    printf("\n");

    return 0;
}