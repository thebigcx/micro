#include <stdio.h>

int fputs(const char* str, FILE* stream)
{
    fwrite(str, strlen(str), 1, stream);
}