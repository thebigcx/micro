#include <stdio.h>
#include <stdlib.h>

FILE* fdopen(int fd, const char* mode)
{
    FILE* file = malloc(sizeof(FILE));

    file->fd  = fd;
    file->eof = 0;
    file->err = 0;

    return file;
}