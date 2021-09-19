#include <stdio.h>

int fgetc(FILE* stream)
{
    char c;
    while (fread(&c, 1, 1, stream) == 0);
    return c;
}