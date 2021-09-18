#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include <string.h>
#include <sys/syscall.h>
#include <errno.h>
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
    //assert(!"atexit() not implemented!\n");
    return 0;
}

int atoi(const char* str)
{
    assert(!"atoi() not implemented!\n");
    return 0;
}

extern char** environ;

char* getenv(const char* name)
{
    char** env = environ;

    while (*env != NULL)
    {
        size_t len = strlen(name);
        char* equ = strchr(*env, '=');

        if (!strncmp(*env, name, len)
          && equ - *env == len)
        {
            return equ + 1;
        }

        env++;
    }

    return NULL;
}

unsigned long strtoul(const char* str, char** endptr, int base)
{
    assert(!"strtoul() not implemented!\n");
    return 0;
}

void qsort(void* base, size_t nitems, size_t size, int (*compar)(const void*, const void*))
{
    assert(!"qsort() is not implemented!\n");
}

char* ptsname(int fd)
{
    static char s_name[128];

    if (ptsname_r(fd, s_name, 128)) return NULL;

    return s_name;
}

int ptsname_r(int fd, char* buf, size_t buflen)
{
    int e = syscall(SYS_ptsname, fd, buf, buflen);
    
    if (e)
    {
        errno = e;
        return -1;
    }

    return 0;
}