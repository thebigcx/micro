#include <unistd.h>
#include <stdio.h>
#include <dirent.h>

int main(int argc, char** argv)
{
    DIR* dir = opendir(".");

    struct dirent* dirent;
    while ((dirent = readdir(dir)))
    {
        printf("%s\n", dirent->d_name);
    }

    closedir(dir);

    return 0;
}