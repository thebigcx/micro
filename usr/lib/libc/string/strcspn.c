#include <string.h>

size_t strcspn(const char* str, const char* delim)
{
    size_t n = 0;
    while (str[n] != 0)
    {
        for (size_t i = 0; i < strlen(delim); i++)
            if (str[n] == delim[i]) return n;
        n++;
    }

    return n;
}