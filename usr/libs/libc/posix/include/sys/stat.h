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

#define S_ISFIFO(m) (m == 0x1000)
#define S_ISCHR(m)  (m == 0x2000)
#define S_ISDIR(m)  (m == 0x4000)
#define S_ISBLK(m)  (m == 0x6000)
#define S_ISREG(m)  (m == 0x8000)
#define S_ISLNK(m)  (m == 0xa000)
#define S_ISSOCK(m) (m == 0xc000)

int    mkdir(const char* path, mode_t mode);
int    stat(const char* path, struct stat* buf);
int    fstat(int fd, struct stat* buf);
int    lstat(const char* path, struct stat* buf);
int    chmod(const char* pathname, mode_t mode);
mode_t umask(mode_t mask);