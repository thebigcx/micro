/*
 *  Kernel minimal C library
 */

#pragma once

#include <micro/types.h>
#include <micro/platform.h>
#include <micro/heap.h>

typedef __builtin_va_list va_list;

#define va_start(v, l)  __builtin_va_start(v, l)
#define va_end(v)       __builtin_va_end(v)
#define va_arg(v, l)    __builtin_va_arg(v, l)
#define va_copy(d, s)   __builtin_va_copy(d, s)

char* strrev(char* str);
char* itoa(int value, char* str, int base);
char* ultoa(unsigned long n, char* str, int base);
char* strtok_r(char* str, const char* delim, char** saveptr);
void snprintf(const char* format, char* s, size_t n, va_list args);

FORCE_INLINE size_t strlen(const char* str)
{
    for (size_t len = 0;; len++) if (str[len] == 0) return len;
}

FORCE_INLINE size_t strspn(const char* str, const char* delim)
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

FORCE_INLINE size_t strcspn(const char* str, const char* delim)
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

FORCE_INLINE int abs(int n)
{
    if (n < 0) return -n;
    return n;
}

FORCE_INLINE int max(int a, int b)
{
    return a > b ? a : b;
}

FORCE_INLINE int min(int a, int b)
{
    return a < b ? a : b;
}

FORCE_INLINE void memset(void* ptr, unsigned char c, size_t size)
{
    unsigned char* cp = (unsigned char*)ptr;
    while (size--) *cp++ = c;
}

FORCE_INLINE void memcpy(void* dst, const void* src, size_t size)
{
    char* cdst = (char*)dst;
    const char* csrc = (char*)src;
    while (size && size--) *cdst++ = *csrc++;
}

FORCE_INLINE int strncmp(const char* s1, const char* s2, size_t n)
{
    while (n--)
    {
        if (*s1 != *s2) return *s1 - *s2;
        s1++; s2++;
    }
    return 0;
}

FORCE_INLINE int strcmp(const char* s1, const char* s2)
{
    if (strlen(s1) != strlen(s2)) return 1;
    return strncmp(s1, s2, strlen(s1));
}

FORCE_INLINE char* strcpy(char* dst, const char* src)
{
    size_t i = 0;
    for (; src[i] != 0; i++)
        dst[i] = src[i];
    dst[i] = 0;

    return dst;
}

FORCE_INLINE char* strncpy(char* dst, const char* src, size_t size)
{
    size_t i = 0;
    for (; i < size; i++)
        dst[i] = src[i];
    dst[i] = 0;

    return dst;
}

FORCE_INLINE char* strdup(const char* s)
{
    char* news = kmalloc(strlen(s) + 1);
    strcpy(news, s);
    return news;
}

FORCE_INLINE void* memmove(void* dst, const void* src, size_t n)
{
    void* imm = kmalloc(n);

    memcpy(imm, src, n);
    memcpy(dst, imm, n);

    kfree(imm);
    return dst;
}

FORCE_INLINE void* memdup(const void* src, size_t n)
{
    void* dst = kmalloc(n);
    memcpy(dst, src, n);
    return dst;
}

extern void __assertion_failed(const char* expr, const char* file, int line);

#define ASSERT(expr) if (!(expr)) __assertion_failed(#expr, __FILE__, __LINE__);