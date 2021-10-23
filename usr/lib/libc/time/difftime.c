#include <time.h>
#include <stdlib.h>

double difftime(time_t t1, time_t t2)
{
    return abs(t2 - t1);
}