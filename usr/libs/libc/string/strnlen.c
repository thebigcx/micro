#include <string.h>

size_t strnlen(const char* str, size_t n)
{
    size_t s = 0;
    while (str[s] && s < n) s++;
    return s;
}