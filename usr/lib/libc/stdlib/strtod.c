#include <stdlib.h>

// TODO: pow() function
int __pow10(int x)
{
    int ret = 1;
    for (int i = 0; i < x; i++) ret *= 10;
    return ret;
}

double strtod(const char* str, char** endptr)
{
    int sign = 1;
    int decimal = 0;
    double ret = 0;
    int flag = 0; // decimal point encountered

    if (*str == '-')
    {
        sign = -1;
        str++;
    }

    while (*str && (isdigit(*str) || *str == '.'))
    {
        if (*str != '.')
        {
            if (flag) decimal++;
            ret = ret * 10 + (*str - '0');
        }
        else
        {
            if (flag) return 0;
            flag = 1;
        }

        str++;
    }

    if (endptr)
        *endptr = str;

    return (double)(ret * sign) / __pow10(decimal);
}