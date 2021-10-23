#include <ctype.h>

int ispunct(int arg)
{
    return (arg >= '!' && arg <= '/') // Based on ASCII table
        || (arg >= ':' && arg <= '@')
        || (arg >= '[' && arg <= '`')
        || (arg >= '{' && arg <= '~');
}