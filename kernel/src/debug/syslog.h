#pragma once

void __sputln(const char*);

#if DEBUG
#define dbgln(s) __sputln(s);
#else
#define dbgln(s);
#endif
