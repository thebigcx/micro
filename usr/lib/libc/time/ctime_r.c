#include <time.h>

char* ctime_r(const time_t* timer, char* buf)
{
    return asctime_r(localtime(timer), buf);
}