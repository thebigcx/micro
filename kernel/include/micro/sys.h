#pragma once

#include <micro/errno.h>
#include <micro/task.h>

#define FDVALID(fd)\
    { if (fd < 0 || fd >= FD_MAX || !task_curr()->fds[fd]) return -EBADF; }

// FD does not exist, but it must be inside 0 - FD_MAX
#define FDRANGEVALID(fd)\
    { if (fd < 0 || fd >= FD_MAX) return -EBADF; }

#define PTRVALID(ptr)\
    { if (!mmu_virt2phys(task_curr()->vm_map, (uintptr_t)ptr)) return -EFAULT; }

// Pointer can be either NULL, or a valid pointer
#define PTRVALIDNULL(ptr)\
    { if (ptr && !mmu_virt2phys(task_curr()->vm_map, (uintptr_t)ptr)) return -EFAULT; }

#define SYSCALL_DEFINE(name, ...)\
    long sys_##name(__VA_ARGS__)

struct dirent;
struct timeval;
struct timezone;
struct stat;

SYSCALL_DEFINE(open,         const char* pathname, uint32_t flags, mode_t mode);
SYSCALL_DEFINE(close,        int fd);
SYSCALL_DEFINE(read,         int fd, void* buf, size_t count);
SYSCALL_DEFINE(write,        int fd, const void* buf, size_t count);
SYSCALL_DEFINE(lseek,        int fdno, off_t offset, int whence);
SYSCALL_DEFINE(access,       const char* pathname, int mode);
SYSCALL_DEFINE(chdir,        const char* path);
SYSCALL_DEFINE(getcwd,       char* buf, size_t size);
SYSCALL_DEFINE(pread,        int fdno, void* buf, size_t size, off_t off);
SYSCALL_DEFINE(pwrite,       int fdno, const void* buf, size_t size, off_t off);
SYSCALL_DEFINE(dup,          int oldfd);
SYSCALL_DEFINE(dup2,         int oldfd, int newfd);
SYSCALL_DEFINE(ioctl,        int fdno, unsigned long req, void* argp);
SYSCALL_DEFINE(mmap,         void* addr, size_t length, int prot,
                             int flags, int fdno, off_t offset);
SYSCALL_DEFINE(munmap,       void* addr, size_t length);
SYSCALL_DEFINE(exit,         int stat);
SYSCALL_DEFINE(waitpid,      int pid, int* wstatus, int options);
SYSCALL_DEFINE(kill,         int pid, int sig);
SYSCALL_DEFINE(insmod,       void* data, size_t len);
SYSCALL_DEFINE(rmmod,        const char* name);
SYSCALL_DEFINE(execve,       const char* path, const char* argv[],
                             const char* envp[]);
SYSCALL_DEFINE(time,         time_t* time);
SYSCALL_DEFINE(ptsname,      int fdno, char* buf, size_t n);
SYSCALL_DEFINE(getdents,     int fdno, struct dirent* dirp, size_t n);
SYSCALL_DEFINE(mount,        const char* src, const char* dst,
                             const char* fs, unsigned long flags,
                             const void* data);
SYSCALL_DEFINE(umount,       const char* target);
SYSCALL_DEFINE(mkdir,        const char* path, mode_t mode);
SYSCALL_DEFINE(gettimeofday, struct timeval* tv, struct timezone* tz);
SYSCALL_DEFINE(ptrace,       unsigned long req, pid_t pid,
                             void* addr, void* data);
SYSCALL_DEFINE(stat,         const char* path, struct stat* buf);
SYSCALL_DEFINE(fstat,        int fd, struct stat* buf);
SYSCALL_DEFINE(lstat,        const char* path, struct stat* buf);
SYSCALL_DEFINE(unlink,       const char* pathname);
SYSCALL_DEFINE(chmod,        const char* pathname, mode_t mode);
SYSCALL_DEFINE(setreuid,     uid_t ruid, uid_t euid);
SYSCALL_DEFINE(chown,        const char* pathname, uid_t uid, uid_t gid);
SYSCALL_DEFINE(readlink,     const char* pathname, char* buf, size_t n);
SYSCALL_DEFINE(getgroups,    int size, gid_t list[]);
SYSCALL_DEFINE(setgroups,    size_t size, const gid_t* list);
SYSCALL_DEFINE(setregid,     gid_t rgid, gid_t egid);
SYSCALL_DEFINE(fork);

void sys_init();