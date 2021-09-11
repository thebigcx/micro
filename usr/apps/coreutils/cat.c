#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("cat: no file specified\n");
        return 0;
    }

    int fd;
    if ((fd = open(argv[1], 0, 0)) < 0)
    {
        printf("cat: %s: no such file or directory\n", argv[1]);
        return 0;
    }

    lseek(fd, 0, SEEK_END);
    size_t len = lseek(fd, 0, SEEK_CUR);
    lseek(fd, 0, SEEK_SET);

    if (!len) return 0;

    char* buffer = malloc(len);
    read(fd, buffer, len);

    for (size_t i = 0; i < len; i++)
        printf("%c", buffer[i]);

    free(buffer);

    return 0;
}
