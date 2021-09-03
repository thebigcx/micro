#include <unistd.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("cat: no file specified\n");
        return 0;
    }

    int fd;
    printf("contents of %s: ", argv[1]);
    if ((fd = open(argv[1], 0, 0)) < 0)
    //if ((fd = open("/initrd/ini", 0, 0)) < 0)
    {
        //printf("cat: no such file or directory\n");
        printf("cat: %s: no such file or directory\n", argv[1]);
        return 0;
    }

    char buffer[200];
    read(fd, buffer, 200);
    printf("%s\n", buffer);

    return 0;
}
