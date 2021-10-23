#include <ctype.h>

int toupper(int arg)
{
    if (arg >= 'a' && arg <= 'z') return arg - 32;
    return arg;
}