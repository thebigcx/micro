#include <string.h>

char* strchr(const char* str, int c)
{
    while (*str != 0)
    {
        if (*str == c) return str;
        str++;
    }
    
    return NULL;
}