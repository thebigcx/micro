#include <stdio.h>
#include <sys/syscall.h>

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("usage: rmmod [name]\n");
        return 0;
    }

    int err = (int)syscall(SYS_rmmod, argv[1]);

    if (err != 0)
    {
        //char buffer[128];
        //strerror_r(err, buffer, 128);
        //printf("rmmod: %s", buffer);
    }

    return 0;
}