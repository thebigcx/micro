#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <bits/confname.h>

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

int link(const char* old, const char* new);
int unlink(const char* pathname);
int symlink(const char* target, const char* link);

int rmdir(const char* path);

void _exit(int status);

pid_t getpid();

pid_t getppid();

int setuid(uid_t uid);
int setgid(gid_t gid);

int seteuid(uid_t euid);
int setegid(gid_t egid);

int setregid(gid_t rgid, gid_t egid);
int setreuid(uid_t ruid, uid_t euid);

uid_t getuid();
uid_t geteuid();

gid_t getgid();
gid_t getegid();

int getgroups(int size, gid_t list[]);

ssize_t readlink(const char* path, char* buf, size_t n);

int pipe(int fds[2]);

char* ttyname(int fd);
char* getlogin();

extern char* optarg;
extern int optind, opterr, optopt;

int getopt(int argc, char* const argv[], const char* optstr);