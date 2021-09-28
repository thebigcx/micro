#include <ctype.h>

int iscntrl(int arg)
{
    return arg < 32 || arg == 127;
}