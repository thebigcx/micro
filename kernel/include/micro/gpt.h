#pragma once

#include <micro/types.h>

struct gpt_header
{
    char sig[8];
    uint32_t rev;
    uint32_t size;
    uint32_t hdr_checksum;
    uint32_t res;
    uint64_t hdrlba;
    uint64_t mirrorlba;
    uint64_t firstblk;
    uint64_t lastblk;
    uint8_t  guid[16];
    uint64_t partarray;
    uint32_t partcnt;
    uint32_t partsize;
    uint32_t array_checksum;
    
    /* blocksize - 0x5c zeroed */
};

struct gpt_entry
{
    uint8_t  type[16];
    uint8_t  partguid[16];
    uint64_t start;
    uint64_t end;
    uint64_t attr;
    uint16_t name[36];
};

int gpt_detect(const char* dev);
int gpt_init(const char* dev);
