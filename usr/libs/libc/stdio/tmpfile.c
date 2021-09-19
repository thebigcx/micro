#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

FILE* tmpfile()
{
    FILE* file = malloc(sizeof(FILE));

    file->fd = open("/tmp", O_RDWR | O_TMPFILE, 0);
    file->err = 0;
    file->eof = 0;

    return file;
}