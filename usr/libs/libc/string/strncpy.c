#include <string.h>

char* strncpy(char* dst, const char* src, uint32_t n)
{
    while (n-- > 0)
    {
        *dst++ = *src++;
    }
    *dst = 0;
    
    return dst;
}