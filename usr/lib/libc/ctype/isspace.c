#include <ctype.h>

int isspace(int arg)
{
    return arg == ' '  || arg == '\n'
        || arg == '\t' || arg == '\v'
        || arg == '\f' || arg == '\r';
}