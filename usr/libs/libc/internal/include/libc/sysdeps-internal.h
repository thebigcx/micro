#pragma once

#include <sys/types.h>
#include <signal.h>
#include <dirent.h>
#include <time.h>

// API: Return value error code, last parameter actual return value

int sys_open(const char* pathname, int flags, mode_t mode, int* fd);
int sys_close(int fd);
int sys_read(int fd, void* buf, size_t cnt, ssize_t* bytes_read);
int sys_write(int fd, const void* buf, size_t cnt, ssize_t* bytes_written);
int sys_exit(int status);
int sys_kill(pid_t pid, int sig);
int sys_sigaction(int signum, const struct sigaction* act, struct sigaction* old);
int sys_fork(pid_t* pid);
int sys_execve(const char* pathname, const char* argv[], const char* envp[]);
int sys_chdir(const char* path);
int sys_getcwd(char* buf, size_t size, char** ret);
int sys_nanosleep(const struct timespec* req, struct timespec* rem);
int sys_lseek(int fd, off_t off, int whence, off_t* npos);
int sys_ioctl(int fd, unsigned long request, void* argp);
int sys_mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset, void** ret);
int sys_munmap(void* addr, size_t length);
int sys_chdir(const char* path);
int sys_getcwd(char* buf, size_t size, char** ret);
int sys_access(const char* pathname, int mode);
int sys_wait(pid_t pid, int* wstatus, int options, pid_t* ret);
int sys_getdents(int fd, struct dirent* dirp, size_t n, ssize_t* bytes);
int sys_mkdir(const char* path, mode_t mode);
int sys_time(time_t* timer);
int sys_dup(int oldfd);
int sys_dup2(int oldfd, int newfd);
int sys_mount(const char* src, const char* dst,
              const char* fstype, unsigned long flags,
              const void* data);
int sys_umount(const char* target);

pid_t sys_getpid();