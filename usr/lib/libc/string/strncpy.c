#include <string.h>

char* strncpy(char* dst, const char* src, uint32_t n)
{
    size_t i;
    for (i = 0; i < n && src[i]; i++)
        dst[i] = src[i];
    for (; i < n; i++) // Pad with zeros
        dst[i] = 0;
        
    return dst;
}