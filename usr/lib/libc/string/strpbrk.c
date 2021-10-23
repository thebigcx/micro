#include <string.h>

char* strpbrk(const char* str1, const char* str2)
{
    return (char*)(str1 + strcspn(str1, str2));
}