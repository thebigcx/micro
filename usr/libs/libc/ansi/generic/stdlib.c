#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include <libc/sysdeps-internal.h>
#include <libc/libc-internal.h>

int abs(int n)
{
    if (n < 0) return -n;
    return n;
}

void abort()
{
    raise(SIGABRT);
}

void exit(int status)
{
	sys_exit(status);
}

int atexit(void (*function)(void))
{
    assert(!"atexit() not implemented!\n");
    return 0;
}

int atoi(const char* str)
{
    assert(!"atoi() not implemented!\n");
    return 0;
}

char* getenv(const char* name)
{
    assert(!"getenv() not implemented\n");
    return NULL;
}
