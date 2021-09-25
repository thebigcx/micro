#include <string.h>

char* strstr(const char* str, const char* substr)
{
    while (*str != 0)
    {
        if (!strcmp(str, substr)) return (char*)str;
        str++;
    }
    return NULL;
}