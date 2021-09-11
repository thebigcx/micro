#pragma once

#include <sys/types.h>
#include <micro/fcntl.h>

int open(const char* pathname, int flags, mode_t mode);
