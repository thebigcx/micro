/*
 *  Kernel minimal C library
 */

#pragma once

#include <types.h>

typedef __builtin_va_list va_list;

#define va_start(v, l)  __builtin_va_start(v, l)
#define va_end(v)       __builtin_va_end(v)
#define va_arg(v, l)    __builtin_va_arg(v, l)
#define va_copy(d, s)   __builtin_va_copy(d, s)

int abs(int n);
size_t strlen(const char* str);
char* strrev(char* str);
char* itoa(int value, char* str, int base);
char* ultoa(unsigned long n, char* str, int base);
void memset(void* ptr, char c, size_t size);
void memcpy(void* dst, const void* src, size_t size);
char* strcpy(char* dst, const char* src);
int strncmp(const char* s1, const char* s2, size_t n);
int strcmp(const char* s1, const char* s2);
char* strtok_r(char* str, const char* delim, char** saveptr);
char* strdup(const char* s);
size_t strspn(const char* str, const char* delim);
size_t strcspn(const char* str, const char* delim);
void snprintf(const char* format, char* s, size_t n, va_list args);