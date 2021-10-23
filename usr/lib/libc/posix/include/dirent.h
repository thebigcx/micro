#pragma once

#include <sys/types.h>

#define DT_UNKNOWN 0
#define DT_REG     1
#define DT_DIR     2
#define DT_CHR     3
#define DT_BLK     4
#define DT_FIFO    5
#define DT_SOCK    6
#define DT_LNK     7

struct dirent
{
    ino_t          d_ino;
    off_t          d_off;
    unsigned short d_reclen;
    unsigned char  d_type;
    char           d_name[256];
};

struct __libc_dir
{
	off_t pos;
	int fd;
};

typedef struct __libc_dir DIR;

struct dirent* readdir(DIR* dirp);
DIR* opendir(const char* name);
int closedir(DIR* dirp);
