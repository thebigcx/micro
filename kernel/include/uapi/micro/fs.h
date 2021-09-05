#pragma once

#define O_RDONLY 0x001
#define O_WRONLY 0x002
#define O_RDWR   0x004

#define O_APPEND 0x008
#define O_CREAT  0x010

#define F_OK     0x000
#define R_OK     0x001
#define W_OK     0x002
#define X_OK     0x004

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

struct dirent
{
    //ino_t d_ino;
    char d_name[128];
};