#include <stdio.h>

int snprintf(char* str, size_t n, const char* format, ...)
{
    va_list list;
    va_start(list, format);

    int ret = vsnprintf(str, n, format, list);
    
    va_end(list);
    return ret;
}