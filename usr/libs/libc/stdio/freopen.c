#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <libc/libc-internal.h>

FILE* freopen(const char* filename, const char* mode, FILE* stream)
{
    close(stream->fd);

    stream->err = 0;
    stream->eof = 0;

    uint32_t flags = __libc_fopen_flags(mode);

    if ((stream->fd = open(filename, flags, 0)) == -1)
        return NULL;

    return stream;
}