#pragma once

#define O_RDONLY    0x000
#define O_WRONLY    0x001
#define O_RDWR      0x002

#define O_CREAT        0100
#define O_EXCL         0200
#define O_NOCTTY       0400
#define O_TRUNC       01000
#define O_APPEND      02000
#define O_NONBLOCK    04000
#define O_DSYNC      010000
#define O_SYNC     04010000
#define O_RSYNC    04010000
#define O_DIRECTORY 0200000
#define O_NOFOLLOW  0400000
#define O_CLOEXEC  02000000

#define O_ASYNC      020000
#define O_DIRECT     040000
#define O_LARGEFILE 0100000
#define O_NOATIME  01000000
#define O_PATH    010000000
#define O_TMPFILE 020200000

#define F_DUPFD         0
#define F_DUPFD_CLOEXEC 1
#define F_GETFD         2
#define F_SETFD         3
#define F_GETFL         4
#define F_SETFL         5
#define F_SETLK         6
#define F_SETLKW        7
#define F_GETLK         8

#define FD_CLOEXEC      0

#define F_RDLCK         0
#define F_WRLCK         1
#define F_UNLCK         2