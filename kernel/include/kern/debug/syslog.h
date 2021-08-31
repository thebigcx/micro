#pragma once
/*
#if DEBUG

void __sputln(const char*);
void __sputlnf(const char*, ...);
void __sputln_crit(const char*);
void __sputlnf_crit(const char*, ...);

#define printk(s) __sputln(s)
#define printk(s, ...) __sputlnf(s, __VA_ARGS__)
#define printk_crit(s) __sputln(s)
#define printk_crit(s, ...) __sputlnf_crit(s, __VA_ARGS__)

#else

#define printk(s);
#define printk(s, ...);

#endif
*/

void printk(const char* s, ...);
void printk_crit(const char* s, ...);