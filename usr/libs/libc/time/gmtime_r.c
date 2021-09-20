#include <time.h>

void gmtime_r(const time_t* timer, struct tm* tm)
{
    time_t time = *timer;

    tm->tm_year = (time / 31104000) + 1970;
    time %= 31104000;

    tm->tm_mon  = (time / 2592000);
    time %= 2592000;
    
    tm->tm_mday = (time / 86400);
    time %= 86400;
    
    tm->tm_hour = (time / 3600);
    time %= 3600;
    
    tm->tm_min  = (time / 60);
    time %= 60;
    
    tm->tm_sec  = (time / 1);
    time %= 1;

    // TODO: this is not right - months aren't always 30 days, and months start on different days of the week
    tm->tm_wday = tm->tm_mday % 7;
    tm->tm_yday = tm->tm_mday + (30 * tm->tm_mon);
}