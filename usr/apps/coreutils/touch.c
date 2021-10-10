#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("usage: touch <file>\n");
        return -1;
    }

    int fd;
    if (!(fd = open(argv[1], O_CREAT | O_RDONLY, 0777)))
    {
        perror("touch");
        return -1;
    }

    close(fd);

    return 0;
}