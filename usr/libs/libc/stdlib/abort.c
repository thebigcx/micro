#include <stdlib.h>
#include <signal.h>

void abort()
{
    raise(SIGABRT);
}