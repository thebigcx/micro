#include <stdio.h>
#include <unistd.h>

int fseek(FILE* stream, long int offset, int whence)
{
    off_t off = lseek(stream->fd, offset, whence);
    if (off < 0)
        return off;

    stream->eof = 0;
    return 0;
}