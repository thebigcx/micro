#include <stdio.h>

int vsprintf(char* str, const char* format, va_list args)
{
    // TODO: figure out size required
    return vsnprintf(str, 200, format, args);
}