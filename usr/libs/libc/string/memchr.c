#include <string.h>

void* memchr(const void* str, int c, size_t n)
{
    char* cstr = str;
    while (n--)
    {
        if (*cstr == (char)c)
            return cstr;
        cstr++;
    }

    return NULL;
}