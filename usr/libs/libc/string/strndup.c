#include <string.h>
#include <stdlib.h>

char* strndup(const char* str, size_t size)
{
    size = strlen(str) > size
         ? size : strlen(str);

    char* new = malloc(size);
    strncpy(new, str, size - 1);
    return new;
}