#include <string.h>

char* strcat(char* dst, const char* src)
{
    return strcpy(dst + strlen(dst), src);
}