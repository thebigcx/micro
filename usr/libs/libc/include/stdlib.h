#pragma once

#include <stddef.h>

#define RAND_MAX 0x7FFFFFFF

#define EXIT_SUCCESS  0
#define EXIT_FAILURE -1

void* malloc(size_t size);
void free(void* ptr);
void* realloc(void* ptr, size_t size);
void* calloc(size_t nitems, size_t size);

char* itoa(int value, char* str, int base);
char* ultoa(unsigned long n, char* str, int base);

int atoi(const char* str);
long int atol(const char* str);
double atof(const char* str);

int abs(int n);

unsigned long strtoul(const char* str, char** endptr, int base);
unsigned long long strtoull(const char* str, char** endptr, int base);
long int strtol(const char* str, char** endptr, int base);

int rand();
void srand(unsigned int seed);

void abort();

void exit(int status);
int atexit(void (*function)(void));

char* getenv(const char* name);
int   putenv(char* string);

void qsort(void* base, size_t nitems, size_t size,
           int (*compar)(const void*, const void*));
void* bsearch(const void* key, const void* base, size_t nmemb, size_t size,
              int (*compar)(const void*, const void*));

char* ptsname(int fd);
int   ptsname_r(int fd, char* buf, size_t buflen);

const char* getprogname();
void setprogname(const char* name);

char* realpath(const char* path, char* respath);

int system(const char* command);

size_t mbstowcs(wchar_t* restrict dst, const char* restrict src, size_t n);

char* mktemp(char* template);
char* mkdtemp(char* template);