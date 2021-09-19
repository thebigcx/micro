#include <stdio.h>

int vfprintf(FILE* file, const char* format, va_list args)
{
    // TODO: calculate size required
    size_t len = 300;
    char* str = malloc(len);

    vsnprintf(str, len, format, args);

    write(file->fd, str, strlen(str));
    free(str);

    return 0;
}