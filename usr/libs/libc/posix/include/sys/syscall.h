#pragma once

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
#define SYS_getdents        16
#define SYS_mkdir           17
#define SYS_ioctl           18
#define SYS_time            19
#define SYS_dup             20
#define SYS_dup2            21
#define SYS_insmod          22
#define SYS_rmmod           23
#define SYS_mount           24
#define SYS_umount          25
#define SYS_pread           26
#define SYS_pwrite          27
#define SYS_ptsname         28
#define SYS_gettimeofday    29
#define SYS_ptrace          30

extern long syscall(long sysno, ...);