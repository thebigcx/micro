#include <string.h>

void* memcpy(void* dst, const void* src, size_t n)
{
    unsigned char* cdst = (unsigned char*)dst;
    unsigned char* csrc = (unsigned char*)src;

    for (size_t i = 0; i < n; i++)
        cdst[i] = csrc[i];

    return dst;
}