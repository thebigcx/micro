#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>

// TODO: move to stdio.c
FILE* fopen(const char* path, const char* mode)
{
    // TODO: parse 'mode' string

    int fd;
    if (!(open(path, 0, 0))) return NULL;
    
    FILE* file = malloc(sizeof(FILE));
    file->fd = fd;
    file->eof = 0;
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
    size_t bytes = read(stream->fd, ptr, size * nmemb);
    if (bytes == 0) stream->eof = 1;
    return bytes;
}

size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream)
{
    return write(stream->fd, ptr, size * nmemb);
}

int fseek(FILE* stream, long int offset, int whence)
{
    off_t off = lseek(stream->fd, offset, whence);
    if (off < 0)
        return off;

    stream->eof = 0;
    return 0;
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

int feof(FILE* stream)
{
    return stream->eof;
}

char* fgets(char* str, int n, FILE* stream)
{
    char* ptr = str;

    char c;
    while (fread(&c, 1, 1, stream) == 1)
    {
        if (ptr - str >= n - 1) return str;

        *ptr++ = c;
        if (c == '\n') break;
    }

    *ptr = 0;

    return str;
}

ssize_t getline(char** lineptr, size_t* n, FILE* stream)
{
    static char line[256];
    
    if (feof(stream)) return -1;

    fgets(line, 256, stream);

    char* ptr = strchr(line, '\n');
    if (ptr) *ptr = 0;
    
    ptr = realloc(*lineptr, 256);
    if (!ptr) return -1;

    *lineptr = ptr;
    *n = 256;

    strcpy(*lineptr, line);
    return strlen(line);
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