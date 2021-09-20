#include <time.h>

void localtime_r(const time_t* timer, struct tm* tm)
{
    // TODO: timezones and stuff
    gmtime_r(timer, tm);
}