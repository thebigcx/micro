#include <stdlib.h>
#include <stdint.h>

void* bsearch(const void* key, const void* base, size_t nmemb, size_t size,
              int (*compar)(const void*, const void*))
{
    for (size_t i = 0; i < nmemb * size; i += size)
    {
        const void* ele = (const void*)((uintptr_t)base + i);

        if (!compar(key, ele))
            return (void*)ele;
    }

    return NULL;
}