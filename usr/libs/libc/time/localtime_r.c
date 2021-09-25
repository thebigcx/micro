#include <time.h>

struct tm* localtime_r(const time_t* timer, struct tm* tm)
{
    // TODO: timezones and stuff
    return gmtime_r(timer, tm);
}