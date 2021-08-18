#include <stdlib.h>

void memset(void* ptr, char c, size_t size)
{
    while (size--) *((char*)ptr) = c;
}
