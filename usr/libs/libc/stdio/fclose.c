#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int fclose(FILE* stream)
{
    int ret = close(stream->fd);
    free(stream);
    return ret;
}