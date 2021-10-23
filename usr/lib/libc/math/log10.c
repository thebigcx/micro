#include <math.h>

double log10(double x)
{
    double ret;
    asm (
        "fldlg2\n"
        "fxch %%st(1)\n"
        "fyl2x\n"
        : "=t"(ret)
        : "0"(x)
    );
    return ret;
}