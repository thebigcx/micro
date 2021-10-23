#include <time.h>

static int __libc_month_days[] =
{
    31, // January
    28, // February
    31, // March
    30, // April
    31, // May
    30, // June
    31, // July
    31, // August
    30, // September
    31, // October
    30, // November
    31  // December
};

static int __libc_month_days_running[] =
{
    31, // January
    59, // February
    90, // March
    120, // April
    151, // May
    181, // June
    212, // July
    243, // August
    273, // September
    304, // October
    334, // November
    365  // December
};

struct tm* gmtime_r(const time_t* timer, struct tm* tm)
{
    time_t time = *timer;

    tm->tm_year = (time / 31104000) + 1970;
    time %= 31104000;

    tm->tm_mon  = (time / 2592000) + 1;
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

    for (size_t i = 0; i < tm->tm_mon - 1; i++)
    {
        tm->tm_yday += __libc_month_days_running[i];
    }

    tm->tm_yday += tm->tm_mday;

    return tm;
}