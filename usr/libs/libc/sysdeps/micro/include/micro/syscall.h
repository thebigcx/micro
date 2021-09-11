#pragma once

#include <stdint.h>

#define SYS_open        	0
#define SYS_close       	1
#define SYS_read        	2
#define SYS_write       	3
#define SYS_fork            4
#define SYS_execve          5
#define SYS_exit            6
#define SYS_kill            7
#define SYS_getpid          8
#define SYS_access          9
#define SYS_lseek           10
#define SYS_waitpid         11
#define SYS_mmap            12
#define SYS_munmap          13
#define SYS_chdir           14
#define SYS_getcwd          15
#define SYS_readdir         16
#define SYS_mkdir           17
#define SYS_ioctl           18

uint64_t syscall(uint64_t sysno, ...);
