#include <stdio.h>
#include <string.h>
#include <stdlib.h>

ssize_t getline(char** lineptr, size_t* n, FILE* stream)
{
    static char line[256];
    
    if (feof(stream)) return -1;

    fgets(line, 256, stream);

    char* ptr = strchr(line, '\n');
    if (ptr) *ptr = 0;
    
    ptr = realloc(*lineptr, 256);
    if (!ptr) return -1;

    *lineptr = ptr;
    *n = 256;

    strcpy(*lineptr, line);
    return strlen(line);
}