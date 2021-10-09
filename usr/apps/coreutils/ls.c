#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

int hidden = 0;
int verbose = 0;

void print_verbose(const char* path, const char* name)
{
    char* full = malloc(strlen(path) + strlen(name) + 2);
    strcpy(full, path);
    strcat(full, "/");
    strcat(full, name);

    struct stat buf;
    lstat(full, &buf);

    if      (S_ISREG(buf.st_mode))  printf("-");
    else if (S_ISDIR(buf.st_mode))  printf("d");
    else if (S_ISBLK(buf.st_mode))  printf("b");
    else if (S_ISCHR(buf.st_mode))  printf("c");
    else if (S_ISFIFO(buf.st_mode)) printf("f");
    else if (S_ISLNK(buf.st_mode))  printf("l");
    else if (S_ISSOCK(buf.st_mode)) printf("s");

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

    printf(" %d", buf.st_nlink);

    // TODO: user
    printf(" root root");

    printf(" %*d", 6, buf.st_size);

    char timestr[32];
    strftime(timestr, 32, "%b %d %H:%M", localtime(&buf.st_mtime));

    printf(" %s", timestr);

    printf(" %s", name);

    if (S_ISLNK(buf.st_mode))
    {
        char link[128];
        readlink(full, link, 128);
        printf(" -> %s", link);
    }
    printf("\n");

    free(full);
}

int main(int argc, char** argv)
{
    char opt;
    while ((opt = getopt(argc, argv, "al")) != -1)
    {
        switch (opt)
        {
            case 'a':
                hidden = 1;
                break;
            case 'l':
                verbose = 1;
                break;
            default:
                return -1;
        }
    }

    size_t i = 1;
    while (i < argc && argv[i][0] == '-') i++;

    const char* path;
    if (i == argc)
        path = ".";
    else
        path = argv[i];

    DIR* dir = opendir(path);
    if (!dir)
    {
        perror("ls: ");
        return -1;
    }

    struct dirent* dirent;
    while ((dirent = readdir(dir)))
    {
        if (dirent->d_name[0] == '.' && !hidden)
            continue;

        if (!verbose)
            printf("%s\n", dirent->d_name);
        else
            print_verbose(path, dirent->d_name);
    }

    closedir(dir);

    return 0;
}
