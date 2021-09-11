#include <ctype.h>

int isspace(int arg)
{
    return arg == ' '  || arg == '\n'
        || arg == '\t' || arg == '\v'
        || arg == '\f' || arg == '\r';
}

int isprint(int arg)
{
    return arg >= 32;
}

int isdigit(int arg)
{
    return arg >= '0' && arg <= '9';
}