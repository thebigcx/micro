#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

int main(int argc, char** argv)
{
    assert(argc > 1);

    int fd;
    if (!(fd = open(argv[1], O_CREAT | O_RDONLY, 0)))
    {
        printf("touch: %s: no such file or directory\n", argv[1]);
    }

    close(fd);

    return 0;
}