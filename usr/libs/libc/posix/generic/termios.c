#include <termios.h>

int tcsetattr(int fd, int optional_actions,
              const struct termios* termios_p)
{
    return 0;
}

int tcgetattr(int fd, struct termios* termios_p)
{
    return 0;
}