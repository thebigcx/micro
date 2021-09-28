#include <unistd.h>
#include <stdio.h>
#include <dirent.h>

int hidden = 0;

int main(int argc, char** argv)
{
    char opt;
    while ((opt = getopt(argc, argv, "a")) != -1)
    {
        switch (opt)
        {
            case 'a':
                hidden = 1;
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

        printf("%s\n", dirent->d_name);
    }

    closedir(dir);

    return 0;
}