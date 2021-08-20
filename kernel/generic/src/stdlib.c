#include <stdlib.h>

void memset(void* ptr, char c, size_t size)
{
    while (size--) *((char*)ptr++) = c;
}

void memcpy(void* dst, const void* src, size_t size)
{
    while (size--) *((char*)dst++) = *((char*)src++);
}

int strncmp(const char* s1, const char* s2, size_t n)
{
    while (n--)
    {
        if (*s1 != *s2) return *s1 - *s2;
        s1++; s2++;
    }
    return 0;
}
