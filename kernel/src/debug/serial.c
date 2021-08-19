#include <debug/syslog.h>
#include <stdlib.h>
#include <types.h>

#if DEBUG

static int abs(int n)
{
    if (n < 0) return -n;
    return n;
}

static size_t strlen(const char* str)
{
    for (size_t len = 0;; len++) if (str[len] == 0) return len;
}

static char* strrev(char* str)
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

static char* itoa(int value, char* str, int base)
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

static char* ultoa(unsigned long n, char* str, int base)
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

static void __sprintf(const char* format, char* s, size_t n, va_list args)
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

#define PORT 0x3f8

static void outb(uint16_t port, uint8_t val)
{
    asm volatile ("outb %0, %1" :: "a"(val), "Nd"(port));
}

static uint8_t inb(uint16_t port)
{
    uint8_t v;
    asm volatile ("inb %1, %0" : "=a"(v) : "Nd"(port));
    return v;
}

static void serial_putch(char c)
{
    static int ready = 0;
    if (!ready)
    {
        outb(PORT + 1, 0x00);
        outb(PORT + 3, 0x80);
        outb(PORT + 0, 0x03);
        outb(PORT + 1, 0x00);
        outb(PORT + 3, 0x03);
        outb(PORT + 2, 0xc7);
        outb(PORT + 4, 0x0b);
        outb(PORT + 4, 0x1e);
        outb(PORT + 0, 0xae);
        outb(PORT + 4, 0x0f);
        ready = 1;
    }

    // Wait for trasmit to be empty
    while ((inb(PORT + 5) & 0x20) == 0);
    outb(PORT, c); 
}

void __sputln(const char* s)
{
    while (*s != 0) serial_putch(*s++);
    serial_putch('\n');
}

void __sputlnf(const char* s, ...)
{
    char buffer[200];

    va_list list;
    va_start(list, s);
    __sprintf(s, buffer, 200, list);
    va_end(list);

    __sputln(buffer);
}

#endif
