#pragma once

#include <micro/types.h>

#define ELF_NIDENT 16

struct elf_hdr
{
    int8_t   ident[ELF_NIDENT];
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint64_t entry;
    uint64_t ph_off;
    uint64_t sh_off;
    uint32_t flags;
    uint16_t eh_size;
    uint16_t ph_ent_size;
    uint16_t ph_num;
    uint16_t sh_ent_size;
    uint16_t sh_num;
    uint16_t sh_str_idx;
};

struct elf_phdr
{
    uint32_t type;
    uint32_t flags;
    uint64_t offset;
    uint64_t vaddr;
    uint64_t paddr;
    uint64_t filesz;
    uint64_t memsz;
    uint64_t align;
};

#define PT_NULL 0
#define PT_LOAD 1