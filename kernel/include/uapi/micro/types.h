#pragma once

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
typedef unsigned long  uint64_t;

typedef char  int8_t;
typedef short int16_t;
typedef int   int32_t;
typedef long  int64_t;

typedef unsigned long uintptr_t;
typedef long intptr_t;

typedef long int intmax_t;
typedef long unsigned int uintmax_t;

typedef long int            ssize_t;
typedef unsigned long int   size_t;
typedef long int            off_t;
typedef unsigned int        mode_t;
typedef int                 pid_t;
typedef unsigned long		ino_t;
typedef unsigned long 		dev_t;
typedef unsigned long		nlink_t;
typedef unsigned int        uid_t;
typedef unsigned int        gid_t;
typedef long				blksize_t;
typedef long				blkcnt_t;
typedef long int            time_t;
typedef uint32_t            sigset_t;

#define INT8_MAX  0x7F
#define INT16_MAX 0x7FFF
#define INT32_MAX 0x7FFFFFFF
#define INT64_MAX 0x7FFFFFFFFFFFFFFF

#define INT8_MIN  -INT8_MAX - 1
#define INT16_MIN -INT16_MAX - 1
#define INT32_MIN -INT32_MAX - 1
#define INT64_MIN -INT64_MAX - 1
#define INT_MIN   -INT32_MAX - 1

#define UINT8_MAX  0xFF
#define UINT16_MAX 0xFFFF
#define UINT32_MAX 0xFFFFFFFF
#define UINT64_MAX 0xFFFFFFFFFFFFFFFF

#define RAND_MAX INT32_MAX

#define NULL (void*)0
