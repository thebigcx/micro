#include <string.h>
#include <stdlib.h>

char* strndup(const char* str, size_t size)
{
    size_t n = strnlen(str, size);
    char* new = malloc(n + 1);
    memcpy(new, str, n);
    new[n] = 0;
    return new;
}