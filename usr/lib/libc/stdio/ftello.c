#include <stdio.h>

off_t ftello(FILE* stream)
{
    return (off_t)ftell(stream);
}