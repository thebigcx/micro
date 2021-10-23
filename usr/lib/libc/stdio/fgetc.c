#include <stdio.h>

int fgetc(FILE* stream)
{
    char c;
    if (fread(&c, 1, 1, stream) != 1)
        return EOF;
    return c;
}