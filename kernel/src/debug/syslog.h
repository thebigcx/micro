#pragma once

#if DEBUG

void __sputln(const char*);
void __sputlnf(const char*, ...);

#define dbgln(s) __sputln(s)
#define dbglnf(s, ...) __sputlnf(s, __VA_ARGS__)

#else

#define dbgln(s);
#define dbglnf(s);

#endif
