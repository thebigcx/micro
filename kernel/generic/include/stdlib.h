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

void memset(void* ptr, char c, size_t size);
void memcpy(void* dst, const void* src, size_t size);
int strncmp(const char* s1, const char* s2, size_t n);
