#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

// TODO: use 'base' parameter
long int strtol(const char* str, char** endptr, int base)
{
    if (base != 10)
    {
        assert(!"strtol() with base other than 10 not implemented!\n");
        return 0;
    }

    long int ret = 0;
    int sign = 1;

    if (*str == '-')
    {
        sign = -1;
        str++;
    }

    for (; *str != 0; str++)
    {
        if (!isdigit(*str)) break;

        ret = ret * 10 + (*str - '0');
    }

    if (endptr)
        *endptr = (char*)str;

    return ret * sign;
}