#include <time.h>

char* asctime(const struct tm* timeptr)
{
    static char buf[26];
    return asctime_r(timeptr, buf);
}