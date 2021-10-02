#include <string.h>

char* strncat(char* dst, const char* src, size_t n)
{
    return strncpy(dst + strlen(dst), src, n);
}