#include <micro/stdlib.h>
#include <micro/heap.h>
#include <micro/debug.h>
#include <arch/panic.h>

char* strrev(char* str)
{
    int i = strlen(str) - 1, j = 0;

    char c;
    while (i > j)
    {
        c = str[i];
        str[i] = str[j];
        str[j] = c;
        i--;
        j++;
    }

    return str;
}

char* itoa(int value, char* str, int base)
{
    if (base < 2 || base > 32)
        return str;

    int n = abs(value);

    int i = 0;
    while (n)
    {
        int r = n % base;

        if (r >= 10)
            str[i++] = 65 + (r - 10);
        else
            str[i++] = 48 + r;

        n = n / base;
    }

    if (i == 0)
        str[i++] = '0';

    if (value < 0 && base == 10)
        str[i++] = '-';

    str[i] = '\0';

    return strrev(str);
}

char* ultoa(unsigned long n, char* str, int base)
{
    if (base < 2 || base > 32)
        return str;

    int i = 0;
    while (n)
    {
        int r = n % base;

        if (r >= 10)
            str[i++] = 65 + (r - 10);
        else
            str[i++] = 48 + r;

        n = n / base;
    }

    if (i == 0)
        str[i++] = '0';

    str[i] = '\0';

    return strrev(str);
}

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

void snprintf(const char* format, char* s, size_t n, va_list args)
{
    char buffer[200];
    size_t stridx = 0;

    while (*format != 0 && stridx < n)
    {
        if (*format == '%')
        {
            format++;

            switch (*format)
            {
                case '%':
                    s[stridx++] = '%';
                    break;

                case 'c':
                    s[stridx++] = va_arg(args, int);
                    break;

                case 'i':
                case 'd':
                {
                    itoa(va_arg(args, int), buffer, 10);
                    size_t len = strlen(buffer);
                    
                    for (size_t i = 0; i < len; i++)
                        s[stridx++] = buffer[i];

                    break;
                }

                case 's':
                {
                    char* arg = va_arg(args, char*);
                    size_t len = strlen(arg);

                    for (size_t i = 0; i < len; i++)
                        s[stridx++] = arg[i];
                    
                    break;
                }

                case 'x':
                {
                    ultoa(va_arg(args, unsigned long), buffer, 16);
                    size_t len = strlen(buffer);

                    for (size_t j = 0; j < len; j++)
                        s[stridx++] = buffer[j];

                    break;
                }
            }
        }
        else
        {
            s[stridx++] = *format;
        }
        format++;
    }

    s[stridx] = 0;
}

void __assertion_failed(const char* expr, const char* file, int line)
{
    printk("Assertion failed %s at %s:%d\n", expr, file, line);
    panic("Unrecoverable error");
}