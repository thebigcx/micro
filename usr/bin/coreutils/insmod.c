#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("usage: insmod <filename>\n");
        return 0;
    }

    int fd;
    if ((fd = open(argv[1], O_RDONLY, 0)) == -1)
    {
        perror("insmod");
        return 0;
    }
    
    size_t len = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    void* data = malloc(len);
    if (read(fd, data, len) == -1)
    {
        perror("insmod");
        return 0;
    }

    close(fd);

    syscall(SYS_init_module, data, len);

    return 0;
}
