#include <setjmp.h>

int _setjmp(jmp_buf env)
{
    return setjmp(env);
}