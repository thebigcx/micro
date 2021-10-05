#pragma once

#define NULL (void*)0

typedef unsigned long size_t;
typedef long ptrdiff_t;
typedef int wchar_t;

#define offsetof(st, mem) __builtin_offsetof(st, mem)