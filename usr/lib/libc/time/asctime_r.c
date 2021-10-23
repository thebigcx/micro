#include <time.h>

// Re-entrant version of asctime()
// Returns date and time in the format:
// Www Mmm dd hh:mm:ss yyyy

char* asctime_r(const struct tm* timeptr, char* buf)
{
    strftime(buf, 26, "%a %b %d %H:%M:%S %Y\n", timeptr);
    return buf;
}