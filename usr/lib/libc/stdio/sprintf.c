#include <stdio.h>

int sprintf(char* str, const char* format, ...)
{
    va_list list;
    va_start(list, format);

    // TODO
    int ret = vsprintf(str, format, list);

    va_end(list);

	return ret;
}