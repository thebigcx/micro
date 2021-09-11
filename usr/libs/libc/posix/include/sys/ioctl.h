#pragma once

#include <micro/ioctls.h>

int ioctl(int fd, unsigned long request, void* argp);