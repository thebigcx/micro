#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

static uint32_t fopen_flags(const char* str)
{
    uint32_t flags = 0;
    
    while (*str != 0)
    {
        switch (*str)
        {
            case 'r':
                flags |= (*(str + 1) == '+' ? O_RDWR : O_RDONLY);
                break;
            case 'w':
                flags |= O_TRUNC | O_CREAT;
                flags |= (*(str + 1) == '+' ? O_RDWR : O_WRONLY);
                break;
            case 'a':
                flags |= O_APPEND | O_CREAT;
                flags |= (*(str + 1) == '+' ? O_RDWR : O_WRONLY);
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
    if ((fd = open(path, flags, 0)) < 0) return NULL;
    
    FILE* file = malloc(sizeof(FILE));
    file->fd   = fd;
    file->eof  = 0;
    file->err  = 0;
    return file;
}