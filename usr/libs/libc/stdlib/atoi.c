#include <stdlib.h>
#include <ctype.h>

// TODO: implement ATOI
int atoi(const char* str)
{
    int ret  = 0;
    int sign = 1;

    if (*str == '-')
    {
        sign = -1;
        str++;
    }

    for (; *str != 0; str++)
    {
        if (!isdigit(*str)) return 0;

        ret = ret * 10 + (*str - '0');
    }

    return ret * sign;
}