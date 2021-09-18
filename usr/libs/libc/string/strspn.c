#include <string.h>

size_t strspn(const char* str, const char* delim)
{
    size_t n = 0;
    while (str[n] != 0)
    {
        for (size_t i = 0; i < strlen(delim); i++)
        {
            if (str[n] == delim[i]) break;
            return n;
        }

        n++;
    }

    return n;
}