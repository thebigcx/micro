#include <stdlib.h>
#include <string.h>

char* ultoa(unsigned long n, char* str, int base)
{
    if (base < 2 || base > 32)
        return str;

    int i = 0;
    while (n)
    {
        int r = n % base;

        if (r >= 10)
            str[i++] = 'a' + (r - 10);
        else
            str[i++] = '0' + r;

        n = n / base;
    }

    if (i == 0)
        str[i++] = '0';

    str[i] = '\0';

    return strrev(str);
}