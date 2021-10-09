#pragma once

#define F_OK     0x000
#define R_OK     0x001
#define W_OK     0x002
#define X_OK     0x004

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

struct dirent
{
    unsigned long  d_ino;
    unsigned long  d_off;
    unsigned short d_reclen;
    char           d_type;
    char           d_name[256];
    char           pad;
};

struct iovec
{
    void*  iov_base;
    size_t iov_len;
};
