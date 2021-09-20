#pragma once

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>

// FIXME:
#include <sys/types.h>

#define BUFSIZ 4096

#define _IOFBF 0
#define _IOLBF 1
#define _IONBF 2

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define EOF -1

#define FILENAME_MAX 255

typedef struct FILE
{
    int fd;
    int eof;
    int err;

} FILE;

extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

int sprintf(char* str, const char* format, ...);
int snprintf(char* str, size_t n, const char* format, ...);
int vsprintf(char* str, const char* format, va_list args);
int vsnprintf(char* str, size_t n, const char* format, va_list args);
int printf(const char* format, ...);
int fprintf(FILE* file, const char* format, ...);
int vfprintf(FILE* file, const char* format, va_list args);

FILE* fopen(const char* path, const char* mode);
FILE* freopen(const char* path, const char* mode, FILE* stream);
int fclose(FILE* stream);
size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream);
size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream);
int fseek(FILE* stream, long int offset, int whence);
int fflush(FILE* stream);
long int ftell(FILE* stream);
void setbuf(FILE* stream, char* buf);
int  setvbuf(FILE* stream, char* buf, int mode, size_t size);
ssize_t getline(char** lineptr, size_t* n, FILE* stream);

char* fgets(char* str, int n, FILE* stream);
int   fgetc(FILE* stream);

int ungetc(int c, FILE* stream);

int feof(FILE* stream);
int ferror(FILE* stream);
int fileno(FILE* stream);

void rewind(FILE* stream);

int fputs(const char* str, FILE* stream);
int fputc(int c, FILE* stream);
void putchar(char c);
void puts(const char* str);

#define putc(c, stream) fputc((c), (stream))

int sscanf(const char* str, const char* format, ...);

void perror(const char* s);

int remove(const char* filename);

FILE* tmpfile();