#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int rmfile(const char* name)
{
    printf("rm: %s\n", name);

    struct stat buf;
    if (stat(name, &buf))
    {
        return -1;
    }

    if (S_ISDIR(buf.st_mode))
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

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("usage: rm <filename>\n");
        return -1;
    }

    if (rmfile(argv[1]))
    {
        perror("rm: ");
        return -1;
    }

    return 0;
}