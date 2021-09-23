#pragma once

#include <sys/types.h>
#include <time.h>

struct stat
{
	dev_t 		st_dev;
	ino_t 		st_ino;
	mode_t 		st_mode;
	nlink_t 	st_nlink;
	uid_t 		st_uid;
	gid_t 		st_gid;
	dev_t 		st_rdev;
	off_t 		st_size;
	blksize_t 	st_blksize;
	blkcnt_t 	st_blocks;
	time_t      st_atime;
	time_t      st_mtime;
	time_t      st_ctime;
};

// Macros to test 'st_mode'

#define S_IFMT      (0xf000)

#define S_ISFIFO(m) ((m & S_IFMT) == 0x1000)
#define S_ISCHR(m)  ((m & S_IFMT) == 0x2000)
#define S_ISDIR(m)  ((m & S_IFMT) == 0x4000)
#define S_ISBLK(m)  ((m & S_IFMT) == 0x6000)
#define S_ISREG(m)  ((m & S_IFMT) == 0x8000)
#define S_ISLNK(m)  ((m & S_IFMT) == 0xa000)
#define S_ISSOCK(m) ((m & S_IFMT) == 0xc000)

#define S_IRWXU 00700
#define S_IRUSR 00400
#define S_IWUSR 00200
#define S_IXUSR 00100

#define S_IRWXG 00070
#define S_IRGRP 00040
#define S_IWGRP 00020
#define S_IXGRP 00010

#define S_IRWXO 00007
#define S_IROTH 00004
#define S_IWOTH 00002
#define S_IXOTH 00001

#define S_ISUID 04000
#define S_ISGID 02000
#define S_ISVTX 01000

int    mkdir(const char* path, mode_t mode);
int    stat(const char* path, struct stat* buf);
int    fstat(int fd, struct stat* buf);
int    lstat(const char* path, struct stat* buf);
int    chmod(const char* pathname, mode_t mode);
mode_t umask(mode_t mask);