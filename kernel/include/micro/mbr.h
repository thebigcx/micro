#pragma once

#include <micro/types.h>
#include <micro/platform.h>

#define MBR_ACTIVE (1 << 7)

struct PACKED mbrent
{
    uint8_t  attr;
    uint8_t  startchs[3];
    uint8_t  type;
    uint8_t  endchs[3];
    uint32_t start;
    uint32_t size;
};

struct PACKED mbr
{
    uint32_t      diskid;
    uint16_t      res;
    struct mbrent parts[4];
    uint16_t      sig; // 0xaa55
};

void mbr_init(const char* dev);
