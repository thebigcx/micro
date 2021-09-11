#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>

// TODO: move to stdio.c
FILE* fopen(const char* path, const char* mode)
{
    // TODO: parse 'mode' string
    FILE* file = malloc(sizeof(FILE));
    file->fd = open(path, 0, 0);
    return file;
}

int fclose(FILE* stream)
{
    int ret = close(stream->fd);
    free(stream);
    return ret;
}

size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream)
{
    return read(stream->fd, ptr, size * nmemb);
}

size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream)
{
    return write(stream->fd, ptr, size * nmemb);
}

int fseek(FILE* stream, long int offset, int whence)
{
    return lseek(stream->fd, offset, whence);
}

int fflush(FILE* stream)
{
    // TODO: buffering
    return 0;
}

long int ftell(FILE* stream)
{
    return lseek(stream->fd, 0, SEEK_CUR);        
}

// TODO: implement stdio buffering
void setbuf(FILE* stream, char* buf)
{
    assert(!"setbuf() not implemented!\n");
}

size_t getline(char** lineptr, size_t* n, FILE* stream)
{
    size_t i = 0;
    char c;

    while (fread(&c, 1, 1, stream) == 1)
    {
        if (c == '\n')
        {
            (*lineptr)[i] = 0;
            return i;
        }
        (*lineptr)[i++] = c;
    }

    return i;
}

int sscanf(const char* str, const char* format, ...)
{
    return 0;
}

void perror(const char* s)
{
    // TODO: implement
    printf("%s\n", s);
}