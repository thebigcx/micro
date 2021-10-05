#include <stdio.h>

int getc_unlocked(FILE* file)
{
    return getc(file);
}