#include <signal.h>
#include <errno.h>

int sigismember(const sigset_t* set, int signum)
{
    if (signum < 0 || signum >= 32)
    {
        errno = -EINVAL;
        return -1;
    }

    return *set & (1 << signum);
}