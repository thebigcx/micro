#include <stdio.h>
#include <errno.h>
#include <string.h>

void perror(const char* s)
{
    char buf[32];
    strerror_r(errno, buf, 32);

    printf("%s%s\n", s, buf);
}