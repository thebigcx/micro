#pragma once

#include <stddef.h>
#include <stdint.h>

void*  memcpy(void* dst, const void* src, size_t n);
void*  memmove(void* dst, const void* src, size_t n);
void*  memset(void* dst, unsigned char c, size_t n);
int    memcmp(const void* s1, const void* s2, size_t n);
void*  memchr(const void* str, int c, size_t n);
size_t strlen(const char* str);
char*  strrev(char* str);
int    strcmp(const char* str1, const char* str2);
int    strncmp(const char* str1, const char* str2, size_t n);
char*  strcpy(char* dst, const char* src);
char*  strncpy(char* dst, const char* src, uint32_t n);
char*  strsep(char** str, const char* delim);
char*  strtok_r(char* str, const char* delim, char** saveptr);
char*  strdup(const char* s);
size_t strspn(const char* str, const char* delim);
size_t strcspn(const char* str, const char* delim);
char*  strchr(const char* str, int c);
char*  strstr(const char* str, const char* substr);
char*  strerror(int errnum);
char*  strerror_r(int errnum, char* buf, size_t len);
char*  strrchr(const char* str, int c);
char*  strcat(char* dst, const char* src);
char*  strndup(const char* str, size_t size);
char*  strpbrk(const char* str1, const char* str2);

// POSIX compatibility (see strings.h)
int    strcasecmp(const char* str1, const char* str2);
int    strncasecmp(const char* str1, const char* str2, size_t n);