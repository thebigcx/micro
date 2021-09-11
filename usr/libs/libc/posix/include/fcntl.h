#pragma once

#include <sys/types.h>
#include <abi-bits/abi.h>

#define O_RDONLY __LIBC_O_RDONLY
#define O_WRONLY __LIBC_O_WRONLY
#define O_RDWR   __LIBC_O_RDWR

#define O_APPEND __LIBC_O_APPEND
#define O_CREAT  __LIBC_O_CREAT

int open(const char* pathname, int flags, mode_t mode);
