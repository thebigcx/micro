#include <stdio.h>

int printf(const char* format, ...)
{
    va_list list;
    va_start(list, format);

    int ret = vfprintf(stdout, format, list);

    va_end(list);

    return ret;
}