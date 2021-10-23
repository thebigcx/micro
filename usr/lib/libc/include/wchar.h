#pragma once

#include <sys/types.h>

typedef unsigned long wctype_t;
typedef long int wint_t;

typedef struct
{
    unsigned char bytes[4];
} mbstate_t;

size_t mbrtowc(wchar_t* restrict pwc, const char* restrict s, size_t n,
               mbstate_t* restrict ps);
size_t mbsrtowcs(wchar_t* restrict dst, const char** restrict src,
                 size_t len, mbstate_t* restrict ps);
size_t mbrlen(const char* restrict s, size_t n, mbstate_t* restrict ps);