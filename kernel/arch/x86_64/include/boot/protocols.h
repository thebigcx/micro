#pragma once

#include <types.h>

struct __attribute__((packed)) st2_tag
{
    uint64_t id;
    uint64_t next;
};

struct __attribute__((packed)) st2header
{
    uint64_t entry;
    uint64_t stack;
    uint64_t flags;
    uint64_t tags;
};

#define ST2_FB_ID 0x3ecc1bc43d0f7971

struct __attribute__((packed)) st2_header_fb
{
    struct st2_tag tag;
    uint16_t width;
    uint16_t height;
    uint16_t bpp;
};

#define ST2_TAG_FB_ID 0x506461d2950408fa

struct __attribute__((packed)) st2struct
{
    char bl_brand[64]; // Bootloader brand
    char bl_vers[64];  // Bootloader version

    uint64_t tags;
};

struct __attribute__((__packed__)) st2_fbinfo
{
    struct st2_tag tag;
    uint64_t addr;
    uint16_t width;
    uint16_t height;
    uint16_t pitch;
    uint16_t depth;
    uint8_t  memmodel;
    uint8_t  rmask_sz;
    uint8_t  rmask_shft;
    uint8_t  gmask_sz;
    uint8_t  gmask_shft;
    uint8_t  bmask_sz;
    uint8_t  bmask_shft;
};

#define ST2_TAG_RSDP_ID 0x9e1786930a375e78

struct __attribute__((packed)) st2_tag_rsdp
{
    struct st2_tag tag;
    uint64_t rsdp;
};
