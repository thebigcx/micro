#include <stdio.h>

int fseeko(FILE* stream, off_t off, int whence)
{
    return fseek(stream, (long int)off, whence);
}