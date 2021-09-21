#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define STATE_NORM  0
#define STATE_FLAG  1
#define STATE_LEN   2
#define STATE_CONV  3

#define IF_HEX    1
#define IF_SIGNED 2
#define IF_OCTAL  4
#define IF_UPPER  8 // Used for the capital X format

#define L_CHAR  1
#define L_SHORT 2
#define L_LONG  3
#define L_LLONG 4

static size_t __fmtint(char* str, size_t n, long long int num, unsigned int int_flags)
{
    char buf[32]; // large enough for now

    int base = int_flags & IF_HEX
             ? 16 : int_flags & IF_OCTAL
             ? 8  : 10;

    if (int_flags & IF_SIGNED)
    {
        // TODO: long signed int to string
        itoa(num, buf, base);
    }
    else
    {
        ultoa((unsigned long)num, buf, base);
    }

    size_t len = strlen(buf);
    for (size_t i = 0; i < len && i < n; i++)
    {
        if (isdigit(buf[i]) && (int_flags & IF_UPPER))
            str[i] = toupper(buf[i]);
        else
            str[i] = buf[i];
    }

    return len < n ? len : n;
}

static size_t __fmtstr(char* buf, size_t n, const char* str)
{
    size_t len = strlen(str);
    for (size_t i = 0; i < len && i < n; i++)
        buf[i] = str[i];

    return len < n ? len : n;
}

static size_t __fmtchr(char* str, size_t n, char c)
{
    if (n) str[0] = c;
    return !!n;
}

int vsnprintf(char* str, size_t n, const char* format, va_list list)
{
    if (!n) return 0;

    size_t i = 0;
    int state = STATE_NORM;

    int len = 0;

    for (;;)
    {
        char c = *format;
        if (state == STATE_NORM)
        {
            format++;
            if (c == '%')
            {
                state = STATE_FLAG;
            }
            else
            {
                str[i++] = c;
                if (i >= n) return i;
            }
        }
        else if (state == STATE_FLAG)
        {
            switch (c)
            {
                default: state = STATE_LEN; break;
            }
        }
        else if (state == STATE_LEN)
        {
            switch (c)
            {
                case 'l': len = len == L_LONG  ? L_LLONG : L_LONG; format++; break;
                case 'h': len = len == L_SHORT ? L_CHAR  : L_SHORT; format++; break;
                default: state = STATE_CONV; break;
            }
        }
        else if (state == STATE_CONV)
        {
            if (c == 'd' || c == 'i' || c == 'x' || c == 'o'
             || c == 'X' || c == 'u')
            {
                long long int num;
                unsigned int int_flags = 0;

                // TODO: precision/width
                if (len == L_CHAR)
                    num = va_arg(list, char);
                else if (len == L_SHORT)
                    num = va_arg(list, short);
                else if (len == L_LONG)
                    num = va_arg(list, long);
                else if (len == L_LLONG)
                    num = va_arg(list, long long);
                else
                    num = va_arg(list, int);

                len = 0;

                if (c == 'x' || c == 'X')
                    int_flags |= IF_HEX;
                else if (c == 'd' || c == 'i')
                    int_flags |= IF_SIGNED;
                else if (c == 'o')
                    int_flags |= IF_OCTAL;

                if (c == 'X')
                    int_flags |= IF_UPPER;

                i += __fmtint(str + i, n - i, num, int_flags);
            }
            else if (c == 's')
            {
                i += __fmtstr(str + i, n - i, va_arg(list, const char*));
            }
            else if (c == 'c')
            {
                i += __fmtchr(str + i, n - i, va_arg(list, int));
            }
            else if (c == '%')
            {
                if (n - i) str[i++] = '%';
            }
            else if (c == 'p')
            {
                if (n - i >= 2)
                {
                    strcpy(str + i, "0x");
                    i += 2;
                }
                i += __fmtint(str + i, n - i, va_arg(list, unsigned long), IF_HEX);
            }
            
            format++;
            state = STATE_NORM;
        }

        if (c == 0 || i >= n) break;
    }

    return i;
}

/*int vsnprintf(char* str, size_t n, const char* format, va_list list)
{
    if (!n) return 0;

    uint32_t stridx = 0;
    char buffer[100];
    size_t len = 0;
    const char* arg = NULL;

    while (*format != '\0')
    {
        if (*format == '%')
        {
            format++;
            
            switch (*format)
            {
                case '%':
                    str[stridx++] = '%';
                    break;
                case 'c':
                    str[stridx++] = va_arg(list, int);
                    break;
                case 'i':
                case 'd':
                    itoa(va_arg(list, int), buffer, 10);
                    len = strlen(buffer);

                    for (size_t j = 0; j < len; j++)
                        str[stridx++] = buffer[j];

                    break;
                case 's':
                    arg = va_arg(list, char*);
                    len = strlen(arg);

                    for (size_t i = 0; i < len; i++)
                        str[stridx++] = arg[i];
                    
                    break;
                
                case 'x':
                    ultoa(va_arg(list, long unsigned int), buffer, 16);
                    len = strlen(buffer);

                    for (size_t j = 0; j < len; j++)
                        str[stridx++] = buffer[j];

                    break;
            }
        }
        else
        {
            str[stridx++] = *format;
        }
        format++;
    }

    str[stridx] = '\0';

    return stridx;
}*/