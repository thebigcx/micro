#include <string.h>
#include <stdlib.h>

void* memmove(void* dst, const void* src, size_t n)
{
    void* imm = malloc(n);

    memcpy(imm, src, n);
    memcpy(dst, imm, n);

    free(imm);
    return dst;
}