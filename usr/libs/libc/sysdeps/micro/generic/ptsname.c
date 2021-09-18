#include <stdlib.h>

char* ptsname(int fd)
{
    static char s_name[128];
    if (ptsname_r(fd, s_name, 128)) return NULL;
    return s_name;
}