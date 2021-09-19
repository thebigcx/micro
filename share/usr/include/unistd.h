#pragma once

#include <sys/types.h>
#include <sys/stat.h>

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define F_OK 0
#define R_OK 1
#define W_OK 2
#define X_OK 4

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

ssize_t read(int fd, void* buf, size_t count);
ssize_t write(int fd, const void* buf, size_t count);

ssize_t pread(int fd, void* buf, size_t count, off_t off);
ssize_t pwrite(int fd, const void* buf, size_t count, off_t off);

int close(int fd);
off_t lseek(int fd, off_t offset, int whence);
int execve(const char* pathname, const char* argv[], const char* envp[]);
int execv(const char* path, const char* argv[]);
int execvp(const char* file, const char* argv[]);
pid_t getpid();
pid_t fork();
int chdir(const char* path);
char* getcwd(char* buf, size_t size);
int stat(const char* pathname, struct stat* statbuf);
unsigned int sleep(unsigned int seconds);
int access(const char* pathname, int mode);
int truncate(const char* path, off_t len);
int ftruncate(int fd, off_t len);
int isatty(int fd);

int dup(int oldfd);
int dup2(int oldfd, int newfd);