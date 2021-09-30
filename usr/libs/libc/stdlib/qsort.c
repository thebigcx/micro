#include <stdlib.h>
#include <string.h>

#define SWAP(x, y, size)            \
    do                              \
    {                               \
        void* buf = malloc(size);   \
        memcpy(buf, x, size);       \
        memcpy(x, y, size);         \
        memcpy(y, buf, size);       \
        free(buf);                  \
    } while (0)

void qsort(void* base, size_t nitems, size_t size, int (*compar)(const void*, const void*))
{
    for (size_t i = 0; i < nitems;)
    {
        if (i && compar(base + (i - 1) * size, base + i * size) > 0)
        {
            SWAP(base + (i - 1) * size, base + i * size, size);
            i--;
        }
        else i++;
    }
}