#include <stdio.h>
#include <errno.h>

int fputc(int c, FILE* stream)
{
    ssize_t ret = fwrite(&c, 1, 1, stream);
    if (ret != 1)
    {
        errno = ret;
        return -1;
    }

    return c;
}