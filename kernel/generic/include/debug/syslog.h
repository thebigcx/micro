#pragma once

#if DEBUG

void __sputln(const char*);
void __sputlnf(const char*, ...);
void __sputln_crit(const char*);
void __sputlnf_crit(const char*, ...);

#define dbgln(s) __sputln(s)
#define dbglnf(s, ...) __sputlnf(s, __VA_ARGS__)
#define dbgln_crit(s) __sputln(s)
#define dbglnf_crit(s, ...) __sputlnf_crit(s, __VA_ARGS__)

#else

#define dbgln(s);
#define dbglnf(s);

#endif
