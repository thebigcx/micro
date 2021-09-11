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

int tolower(int arg)
{
    if (arg >= 'A' && arg <= 'Z') return arg + 32;
    return arg;
}

int isalpha(int arg)
{
    return (arg >= 'a' && arg <= 'z')
        || (arg >= 'A' && arg <= 'Z');
}

int isalnum(int arg)
{
    return (arg >= '0' && arg <= '9')
        || (arg >= 'a' && arg <= 'z')
        || (arg >= 'A' && arg <= 'Z');
}

int isxdigit(int arg)
{
    return (arg >= '0' && arg <= '9')
        || (arg >= 'a' && arg <= 'f')
        || (arg >= 'A' && arg <= 'F');
}

int ispunct(int arg)
{
    return (arg >= '!' && arg <= '/') // Based on ASCII table
        || (arg >= ':' && arg <= '@')
        || (arg >= '[' && arg <= '`')
        || (arg >= '{' && arg <= '~');
}

int toupper(int arg)
{
    if (arg >= 'a' && arg <= 'z') return arg - 32;
    return arg;
}