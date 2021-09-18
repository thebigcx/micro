#include <string.h>

char* strtok_r(char* s, const char* delim, char** saveptr)
{
	if (!s) s = *saveptr;

    if (*s == 0)
    {
        *saveptr = s;
        return (char*)NULL;
    }

    s += strspn(s, delim);

    if (*s == 0)
    {
        *saveptr = s;
        return (char*)NULL;
    }

    char* end = s + strcspn(s, delim);

    if (*end == 0)
    {
        *saveptr = end;
        return s;
    }

    *end = 0;
    *saveptr = end + 1;
    return s;
}