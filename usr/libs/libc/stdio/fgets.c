#include <stdio.h>

char* fgets(char* str, int n, FILE* stream)
{
    char* ptr = str;

    char c;
    while ((c = fgetc(stream)) != EOF)
    {
        if (ptr - str >= n - 1) return str;

        *ptr++ = c;
        if (c == '\n')
        {
            *ptr = 0;
            return str;
        }
    }

    return NULL;
}