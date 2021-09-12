#include <unistd.h>
#include <stdio.h>
#include <dirent.h>

int main(int argc, char** argv)
{
    char* path = argc == 1 ? "." : argv[1];

    DIR* dir = opendir(path);

    struct dirent* dirent;
    while ((dirent = readdir(dir)))
    {
        printf("%s\n", dirent->d_name);
    }

    closedir(dir);

    return 0;
}