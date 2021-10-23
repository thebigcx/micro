#include <ctype.h>

int isalpha(int arg)
{
    return (arg >= 'a' && arg <= 'z')
        || (arg >= 'A' && arg <= 'Z');
}