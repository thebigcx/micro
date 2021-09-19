#include <stdio.h>
#include <unistd.h>
#include <errno.h>

size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream)
{
    ssize_t bytes = write(stream->fd, ptr, size * nmemb);

    if (bytes < 0) stream->err = errno;

    return bytes;
}