#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

static int force, recurse;

int rmfile(const char* name)
{
    struct stat buf;
    if (stat(name, &buf))
    {
        return -1;
    }

    if (S_ISDIR(buf.st_mode) && recurse)
    {
        DIR* dir = opendir(name);
        struct dirent* dirent;
        while ((dirent = readdir(dir)))
        {
            if (!strcmp(dirent->d_name, ".") || !strcmp(dirent->d_name, ".."))
                continue;

            char* full = malloc(strlen(name) + strlen(dirent->d_name) + 2);
            strcpy(full, name);
            strcat(full, "/");
            strcat(full, dirent->d_name);
            
            if (rmfile(full)) return -1;
            
            free(full);
        }

        return rmdir(name);
    }
    else
    {
        return unlink(name);
    }
}

void usage()
{
    printf("usage: rm [-rf] <filename>\n");
}

int main(int argc, char** argv)
{
    force = recurse = 0;

    char opt;
    while ((opt = getopt(argc, argv, "rf")) != -1)
    {
        switch (opt)
        {
            case 'f':
                force = 1;
                break;
            case 'r':
                recurse = 1;
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

    if (rmfile(argv[i]))
    {
        perror("rm: ");
        return -1;
    }

    return 0;
}