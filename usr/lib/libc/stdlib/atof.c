#include <stdlib.h>
#include <string.h>

double atof(const char* str)
{
    char* endptr;
    double ret = strtod(str, endptr);
    
    return endptr - str == strlen(str) ? ret : 0;
}