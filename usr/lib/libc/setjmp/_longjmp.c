#include <setjmp.h>

void _longjmp(jmp_buf env, int value)
{
    return longjmp(env, value);
}