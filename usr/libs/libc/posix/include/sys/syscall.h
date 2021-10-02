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
#define SYS_stat            31
#define SYS_fstat           32
#define SYS_lstat           33
#define SYS_unlink          34
#define SYS_chmod           35
#define SYS_setreuid        36
#define SYS_chown           37
#define SYS_readlink        38
#define SYS_getuid          39
#define SYS_geteuid         40
#define SYS_getgid          41
#define SYS_getegid         42
#define SYS_getgroups       43
#define SYS_setgroups       44
#define SYS_setregid        45
#define SYS_symlink         46
#define SYS_link            47
#define SYS_sigaction       48
#define SYS_sigreturn       49
#define SYS_sigprocmask     50
#define SYS_umask           51
#define SYS_pipe            52
#define SYS_rename          53
#define SYS_rmdir           54
#define SYS_reboot          55
#define SYS_uname           56
#define SYS_getppid         57
#define SYS_fchmod          58
#define SYS_mknod           59
#define SYS_setuid          60
#define SYS_setgid          61
#define SYS_fcntl           62
#define SYS_utime           63
#define SYS_utimes          64

extern long syscall(long sysno, ...);