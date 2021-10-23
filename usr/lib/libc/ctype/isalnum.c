#include <ctype.h>

int isalnum(int arg)
{
    return (arg >= '0' && arg <= '9')
        || (arg >= 'a' && arg <= 'z')
        || (arg >= 'A' && arg <= 'Z');
}