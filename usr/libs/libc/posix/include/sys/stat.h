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
	time_t      st_atim;
	time_t      st_mtim;
	time_t      st_ctim;
};

int mkdir(const char* path, mode_t mode);
int stat(const char* path, struct stat* buf);
int fstat(int fd, struct stat* buf);
int lstat(const char* path, struct stat* buf);