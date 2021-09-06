#include <micro/stdlib.h>
#include <micro/heap.h>
#include <micro/debug.h>
#include <arch/panic.h>

int abs(int n)
{
    if (n < 0) return -n;
    return n;
}

size_t strlen(const char* str)
{
    for (size_t len = 0;; len++) if (str[len] == 0) return len;
}

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

void memset(void* ptr, unsigned char c, size_t size)
{
    unsigned char* cp = (unsigned char*)ptr;
    while (size--) *cp++ = c;
}

void memcpy(void* dst, const void* src, size_t size)
{
    char* cdst = (char*)dst;
    const char* csrc = (char*)src;
    while (size && size--) *cdst++ = *csrc++;
}

int strncmp(const char* s1, const char* s2, size_t n)
{
    while (n--)
    {
        if (*s1 != *s2) return *s1 - *s2;
        s1++; s2++;
    }
    return 0;
}

int strcmp(const char* s1, const char* s2)
{
    if (strlen(s1) != strlen(s2)) return 1;
    return strncmp(s1, s2, strlen(s1));
}

char* strcpy(char* dst, const char* src)
{
    size_t i = 0;
    for (; src[i] != 0; i++)
        dst[i] = src[i];
    dst[i] = 0;

    return dst;
}

size_t strspn(const char* str, const char* delim)
{
    size_t n = 0;
    while (str[n] != 0)
    {
        for (size_t i = 0; i < strlen(delim); i++)
        {
            if (str[n] == delim[i]) break;
            return n;
        }

        n++;
    }

    return n;
}

size_t strcspn(const char* str, const char* delim)
{
    size_t n = 0;
    while (str[n] != 0)
    {
        for (size_t i = 0; i < strlen(delim); i++)
            if (str[n] == delim[i]) return n;
        n++;
    }

    return n;
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

char* strdup(const char* s)
{
    char* news = kmalloc(strlen(s) + 1);
    strcpy(news, s);
    return news;
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