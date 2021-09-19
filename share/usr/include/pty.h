#pragma once

#include <termios.h>

int openpty(int* amaster, int* aslave, char* name,
            const struct termios* termp,
            const struct winsize* winp);