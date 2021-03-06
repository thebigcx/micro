#include <string.h>

char* strrchr(const char* str, int c)
{
    if (!strlen(str)) return NULL;
    const char* ptr = str + strlen(str) - 1;

    while (*ptr != c)
    {
        if (ptr == str) return NULL;
        ptr--;
    }

    return (char*)ptr;
}