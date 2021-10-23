#include <ctype.h>

int isxdigit(int arg)
{
    return (arg >= '0' && arg <= '9')
        || (arg >= 'a' && arg <= 'f')
        || (arg >= 'A' && arg <= 'F');
}