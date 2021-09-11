#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

int vsnprintf(char* str, size_t n, const char* format, va_list list)
{
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
}

int vsprintf(char* str, const char* format, va_list args)
{
    return vsnprintf(str, 200, format, args);
}

int snprintf(char* str, size_t n, const char* format, ...)
{
    va_list list;
    va_start(list, format);
    int ret = vsnprintf(str, n, format, list);
    va_end(list);
    return ret;
}

int sprintf(char* str, const char* format, ...)
{
    va_list list;
    va_start(list, format);

    

    va_end(list);

	return 0;
}

int vfprintf(FILE* file, const char* format, va_list args)
{
    // TODO: this is bad
    size_t len = 300;
    char* str = malloc(len);
    vsnprintf(str, len, format, args);
    write(file->fd, str, strlen(str));
    free(str);
    return 0;
}

int fprintf(FILE* file, const char* format, ...)
{
    va_list list;
    va_start(list, format);

    int ret = vfprintf(file, format, list);

    va_end(list);

    return ret;
}

int printf(const char* format, ...)
{
    va_list list;
    va_start(list, format);

    int ret = vfprintf(stdout, format, list);

    va_end(list);

    return ret;
}