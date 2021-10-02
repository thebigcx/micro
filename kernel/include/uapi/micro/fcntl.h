#pragma once

#define O_RDONLY    0x001
#define O_WRONLY    0x002
#define O_RDWR      0x003

#define O_APPEND    0x004
#define O_CREAT     0x008
#define O_TMPFILE   0x010
#define O_TRUNC     0x020
#define O_DIRECTORY 0x040
#define O_EXCL      0x080
#define O_PATH      0x100
#define O_NOFOLLOW  0x200

#define F_DUPFD         0
#define F_DUPFD_CLOEXEC 1
#define F_GETFD         2
#define F_SETFD         3
#define F_GETFL         4
#define F_SETFL         5
#define F_SETLK         6
#define F_SETLKW        7
#define F_GETLK         8
#define F_UNLCK         9

#define FD_CLOEXEC      0