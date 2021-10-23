#pragma once

#include <sys/types.h>

typedef struct
{
    size_t re_nsub;
} regex_t;

typedef struct
{
    regoff_t rm_so;
    regoff_t rm_eo;

} regmatch_t;

#define REG_EXTENDED 0x1
#define REG_ICASE    0x2
#define REG_NOSUB    0x4
#define REG_NEWLINE  0x8

#define REG_NOTBOL   0x1
#define REG_NOTEOL   0x2
#define REG_STARTEND 0x4

int regcomp(regex_t* preg, const char* regex, int cflags);
int regexec(const regex_t* preg, const char* string, size_t nmatch,
            regmatch_t* pmatch, int eflags);
size_t regerror(int errcode, const regex_t* preg, char* errbuf,
                size_t errbuf_size);
void regfree(regex_t* preg);