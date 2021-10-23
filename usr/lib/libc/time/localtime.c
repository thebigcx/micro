#include <time.h>

struct tm* localtime(const time_t* timer)
{
	static struct tm tm;
	localtime_r(timer, &tm);
	return &tm;
}