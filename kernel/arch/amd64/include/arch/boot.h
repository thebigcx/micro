#pragma once

#include <micro/types.h>
#include <micro/platform.h>

struct PACKED st2_tag
{
    uint64_t id;
    uint64_t next;
};

struct PACKED st2header
{
    uint64_t entry;
    uint64_t stack;
    uint64_t flags;
    uint64_t tags;
};

#define ST2_FB_ID 0x3ecc1bc43d0f7971

struct PACKED st2_header_fb
{
    struct st2_tag tag;
    uint16_t width;
    uint16_t height;
    uint16_t bpp;
};

#define ST2_TAG_FB_ID 0x506461d2950408fa

struct PACKED st2struct
{
    char bl_brand[64]; // Bootloader brand
    char bl_vers[64];  // Bootloader version

    uint64_t tags;
};

struct PACKED st2_fbinfo
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

struct PACKED st2_tag_rsdp
{
    struct st2_tag tag;
    uint64_t rsdp;
};

#define ST2_TAG_MODS_ID 0x4b6fe466aade04ce

struct PACKED st2_module
{
    uint64_t begin;
    uint64_t end;

#define ST2_MOD_STRSIZE 128
    char string[ST2_MOD_STRSIZE];
};

struct PACKED st2_tag_mods
{
    struct st2_tag tag;
    uint64_t module_cnt;
    struct st2_module modules[];
};

#define ST2_TAG_MMAP_ID 0x2187f79e8612de07

#define ST2_MMAP_USABLE                 1
#define ST2_MMAP_RES                    2
#define ST2_MMAP_ACPI_RECL              3
#define ST2_MMAP_ACPI_NVS               4
#define ST2_MMAP_BAD_MEM                5
#define ST2_MMAP_BOOTLD_RECL            0x1000
#define ST2_MMAP_KMODS                  0x1001
#define ST2_MMAP_FB                     0x1002

struct PACKED st2_mmap_ent
{
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t unused;
};

struct PACKED st2_tag_mmap
{
    struct st2_tag tag;
    uint64_t entries;
    struct st2_mmap_ent mmap[];
};

#define ST2_TERM_ID 0xa85d499b1823be72

struct PACKED st2_header_term
{
    struct st2_tag tag;
    uint64_t flags;
};

#define ST2_TAG_TERM_ID 0xc2b3f4c3233b0974

struct PACKED st2_tag_term
{
    struct st2_tag tag;
    uint32_t flags;
    uint16_t cols;
    uint16_t rows;
    uint64_t term_write;
};

typedef void (*term_write_t)(const char* , size_t);

#define ST2_TAG_CMDLINE_ID 0xe5e76a1b4597a781

struct PACKED st2_tag_cmdline
{
    struct st2_tag tag;
    uint64_t cmdline;
};

struct bootparams
{
    uintptr_t rsdp;
    uintptr_t initrd_phys_start;
    uintptr_t initrd_phys_end;

    uintptr_t fb_phys_addr;
    unsigned int fbwidth, fbheight, fbbpp;

    char cmdline[128];
};

extern term_write_t term_write;
extern int use_boot_term;

void main(struct bootparams params);