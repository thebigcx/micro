#include <stdlib.h>

void memset(void* ptr, char c, size_t size)
{
    while (size--) *((char*)ptr++) = c;
}

void memcpy(void* dst, const void* src, size_t size)
{
    while (size--) *((char*)dst++) = *((char*)src++);
}
