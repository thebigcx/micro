#pragma once

#define TIOCGWINSZ 0
#define TCSETS     1
#define TCSETSW    2
#define TCSETSF    3
#define TCGETS     4

int ioctl(int fd, unsigned long request, void* argp);