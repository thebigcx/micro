#pragma once

#define _POSIX_VERSION 200809L

typedef long int            ssize_t;
typedef unsigned long int   size_t;
typedef long int            off_t;
typedef unsigned int        mode_t;
typedef int                 pid_t;
typedef unsigned long		ino_t;
typedef unsigned long 		dev_t;
typedef int 				nlink_t;
typedef int					uid_t;
typedef int					gid_t;
typedef long				blksize_t;
typedef long				blkcnt_t;
typedef long int            time_t;
typedef long int            suseconds_t;
typedef long                clock_t;
typedef int                 socklen_t;
typedef long int            regoff_t;

typedef unsigned int        u_int;
typedef unsigned long       u_long;
typedef short               bits16_t;
typedef unsigned short      u_bits16_t;
typedef int                 bits32_t;
typedef unsigned int        u_bits32_t;
typedef char*               bits64_t;