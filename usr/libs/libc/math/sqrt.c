#include <math.h>

double sqrt(double x)
{
    // TODO: move into arch folder
    double ret;
    asm ("fsqrt" : "=t"(ret) : "0"(x));
    return ret;
}