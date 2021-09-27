#include <signal.h>
#include <errno.h>

int sigaddset(sigset_t* set, int signum)
{
    if (signum < 0 || signum >= 32)
    {
        errno = -EINVAL;
        return -1;
    }

    *set |= (1 << signum);
    return 0;
}