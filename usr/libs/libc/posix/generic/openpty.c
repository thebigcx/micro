#include <pty.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

int openpty(int* amaster, int* aslave, char* name,
            const struct termios* termp,
            const struct winsize* winp)
{
    int ptm = open("/dev/ptmx", O_RDONLY, 0);

    char* ptsn = ptsname(ptm);
    int pts = open(ptsn, O_RDWR, 0);

    if (name)
        strcpy(name, ptsn);

    if (termp)
    {
        // TODO
    }

    if (winp)
    {
        // TODO
    }

    *amaster = ptm;
    *aslave  = pts;

    return 0;
}