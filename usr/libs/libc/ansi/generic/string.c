#include <string.h>
#include <stdint.h>
#include <stdlib.h>

int memcmp(const void* s1, const void* s2, size_t n)
{
    unsigned char* cs1 = (unsigned char*)s1;
    unsigned char* cs2 = (unsigned char*)s2;

    for (size_t i = 0; i < n; i++)
    {
        if (cs1[i] < cs2[i]) return -1;
        if (cs1[i] > cs2[i]) return 1;
    }

    return 0;
}

void* memcpy(void* dst, const void* src, size_t n)
{
    unsigned char* cdst = (unsigned char*)dst;
    unsigned char* csrc = (unsigned char*)src;

    for (size_t i = 0; i < n; i++)
        cdst[i] = csrc[i];

    return dst;
}

// TODO: implement precautions of overlapping memory blocks
void* memmove(void* dst, const void* src, size_t n)
{
    unsigned char* cdst = (unsigned char*)dst;
    unsigned char* csrc = (unsigned char*)src;

    for (size_t i = 0; i < n; i++)
        cdst[i] = csrc[i];

    return dst;
}

void* memset(void* dst, unsigned char c, size_t n)
{
    unsigned char* cdst = (unsigned char*)dst;

    for (size_t i = 0; i < n; i++)
        cdst[i] = c;

    return dst;
}

int strcmp(const char* str1, const char* str2)
{
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);
    size_t len = len1 < len2 ? len1 : len2;

    if (len == 0 || len1 != len2) return 1;

    for (size_t i = 0; i < len; i++)
    {
        if (str1[i] != str2[i])
        {
            return str1[i] - str2[i];
        }
    }

    return 0;
}

char* strcpy(char* dst, const char* src)
{
    char* ptr = dst;

    while (*src != '\0')
    {
        *dst = *src;
        dst++;
        src++;
    }

    *dst = '\0';

    return ptr;
}

size_t strlen(const char* str)
{
    size_t s = 0;

    while (str[s] != '\0') s++;

    return s;
}

int strncmp(const char* str1, const char* str2, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        if (str1[i] != str2[i])
        {
            return str1[i] - str2[i];
        }
    }

    return 0;
}

char* strncpy(char* dst, const char* src, uint32_t n)
{
    while (n--)
    {
        *dst++ = *src++;
    }
    
    return dst;
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

char* strsep(char** str, const char* delim)
{
    char* begin, *end;
    size_t len = strlen(delim);
    int s = 0;

    begin = *str;

    while (1)
    {
        if (begin[s] == '\0') break;

        for (size_t i = 0; i < len; i++)
        {
            if (begin[s] == delim[i]) break;
        }

        s++;
    }

    end = begin + s;

    if (*end)
    {
        *(end++) = '\0';
        *str = end;
    }
    else
    {
        *str = NULL;
    }

    return begin;
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
    char* news = malloc(strlen(s) + 1);
    strcpy(news, s);
    return news;
}

char* strchr(const char* str, int c)
{
    while (*str != 0)
    {
        if (*str == c) break;
        str++;
    }
    return str;
}

char* strstr(const char* str, const char* substr)
{
    while (*str != 0)
    {
        if (!strcmp(str, substr)) return str;
        str++;
    }
    return NULL;
}

char* strerror(int errnum)
{
    return "Error\n";
}