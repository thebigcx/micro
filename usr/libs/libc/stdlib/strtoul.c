#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

unsigned long strtoul(const char* str, char** endptr, int base)
{
    if (base != 10)
    {
        assert(!"strtol() with base other than 10 not implemented!\n");
        return 0;
    }

    unsigned long ret;

    for (ret = 0; *str != 0 && isdigit(*str); str++)
    {
        ret = ret * 10 + (*str - '0');
    }

    if (endptr)
        *endptr = (char*)str;

    return ret;
}