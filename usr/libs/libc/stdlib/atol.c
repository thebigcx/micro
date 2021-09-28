#include <stdlib.h>

long int atol(const char* str)
{
    return strtol(str, NULL, 10);
}