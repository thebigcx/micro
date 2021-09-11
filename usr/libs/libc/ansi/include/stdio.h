#pragma once

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>

// FIXME:
#include <sys/types.h>

#define BUFSIZ 4096

typedef struct FILE
{
    int fd;
    int eof;

} FILE;

extern FILE* stdout;
extern FILE* stdin;
extern FILE* stderr;

void putchar(char c);
void puts(const char* str);

int sprintf(char* str, const char* format, ...);
int snprintf(char* str, size_t n, const char* format, ...);
int vsprintf(char* str, const char* format, va_list args);
int vsnprintf(char* str, size_t n, const char* format, va_list args);
void printf(const char* format, ...);
void fprintf(FILE* file, const char* format, ...);
int vfprintf(FILE* file, const char* format, va_list args);

FILE* fopen(const char* path, const char* mode);
int fclose(FILE* stream);
size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream);
size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream);
int fseek(FILE* stream, long int offset, int whence);
int fflush(FILE* stream);
long int ftell(FILE* stream);
void setbuf(FILE* stream, char* buf);
ssize_t getline(char** lineptr, size_t* n, FILE* stream);
char* fgets(char* str, int n, FILE* stream);
int feof(FILE* stream);

int sscanf(const char* str, const char* format, ...);

void perror(const char* s);