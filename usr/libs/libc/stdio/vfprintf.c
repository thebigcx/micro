#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int vfprintf(FILE* file, const char* format, va_list args)
{
    // TODO: calculate size required
    size_t len = 300;
    char* str = malloc(len);

    int ret = vsnprintf(str, len, format, args);

    write(file->fd, str, strlen(str));
    free(str);

    return ret;
}