#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

FILE _stdin = 
{
    .fd = 0
};

FILE _stdout = 
{
    .fd = 1
};

FILE _stderr = 
{
    .fd = 2
};

FILE* stdout = &_stdout;
FILE* stdin = &_stdin;
FILE* stderr = &_stderr;

static uint32_t fopen_flags(const char* str)
{
    uint32_t flags = 0;

    while (*str != 0)
    {
        switch (*str)
        {
            case 'r':
                flags |= (*str++ == '+' ? O_RDWR : O_RDONLY);
                break;
            case 'w':
                flags |= O_TRUNC | O_CREAT;
                flags |= (*str++ == '+' ? O_RDWR : O_WRONLY);
                break;
            case 'a':
                flags |= O_APPEND | O_CREAT;
                flags |= (*str++ == '+' ? O_RDWR : O_WRONLY);
                break;
        }
        
        str++;
    }

    return flags;
}

// TODO: move to stdio.c
FILE* fopen(const char* path, const char* mode)
{
    uint32_t flags = fopen_flags(mode);

    int fd;
    if ((fd = open(path, flags, 0)) == -1) return NULL;
    
    FILE* file = malloc(sizeof(FILE));
    file->fd   = fd;
    file->eof  = 0;
    file->err  = 0;
    return file;
}

FILE* freopen(const char* path, const char* mode, FILE* stream)
{
    uint32_t flags = fopen_flags(mode);

    int fd;
    if ((fd = open(path, flags, 0)) == -1) return NULL;

    close(stream->fd);

    stream->fd  = fd;
    stream->err = 0;
    stream->eof = 0;

    return stream;
}

int fclose(FILE* stream)
{
    int ret = close(stream->fd);
    free(stream);
    return ret;
}

size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream)
{
    ssize_t bytes = read(stream->fd, ptr, size * nmemb);

    if (bytes == 0) stream->eof = 1;
    else if (bytes < 0) stream->err = errno;

    return bytes;
}

size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream)
{
    ssize_t bytes = write(stream->fd, ptr, size * nmemb);

    if (bytes < 0) stream->err = errno;

    return bytes;
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

int setvbuf(FILE* stream, char* buf, int mode, size_t size)
{
    assert(!"setvbuf() not implemented!\n");
}

int feof(FILE* stream)
{
    return stream->eof;
}

int ferror(FILE* stream)
{
    return stream->err;
}

void rewind(FILE* stream)
{
    fseek(stream, 0, SEEK_SET);
}

int fileno(FILE* stream)
{
    return stream->fd;
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

int fgetc(FILE* stream)
{
    char c;
    if (fread(&c, 1, 1, stream) == 1) return c;
    return EOF;
}

// TODO: ungetc will be possible when I add buffering
int ungetc(int c, FILE* stream)
{
    assert(!"ungetc() is not implemented!\n");
    return -1;
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

void puts(const char* str)
{
    fputs(str, stdout);
}

void putchar(char c)
{
    fputc(c, stdout);
}

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

int fputs(const char* str, FILE* stream)
{
    fwrite(str, strlen(str), 1, stream);
}

int remove(const char* filename)
{
    // TODO: implement
    return 0;
}

FILE* tmpfile()
{
    FILE* file = malloc(sizeof(FILE));

    file->fd = open("/tmp", O_RDWR | O_TMPFILE, 0);
    file->err = 0;
    file->eof = 0;

    return file;
}