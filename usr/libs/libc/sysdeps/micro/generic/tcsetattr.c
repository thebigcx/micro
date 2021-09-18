#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>

int tcsetattr(int fd, int optional_actions,
              const struct termios* termios_p)
{
    switch (optional_actions)
    {
        case TCSANOW:
            return ioctl(fd, TCSETS, termios_p);
        case TCSADRAIN:
            return ioctl(fd, TCSETSW, termios_p);
        case TCSAFLUSH:
            return ioctl(fd, TCSETSF, termios_p);
    }

    errno = EINVAL;
    return -1;
}