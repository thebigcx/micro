#include <ctype.h>

int isgraph(int arg)
{
    return arg > 32 && arg != 127;
}