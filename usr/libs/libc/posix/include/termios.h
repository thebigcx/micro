#pragma once

#include <micro/termios.h>

int tcsetattr(int fd, int optional_actions,
              const struct termios* termios_p);
int tcgetattr(int fd, struct termios* termios_p);