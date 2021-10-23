#include <string.h>

void* memchr(const void* str, int c, size_t n)
{
    const char* cstr = str;
    while (n--)
    {
        if (*cstr == (char)c)
            return (void*)cstr;
        cstr++;
    }

    return NULL;
}