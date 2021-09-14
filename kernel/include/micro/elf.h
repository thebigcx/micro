/*
 *  ELF (Executable and Linker Format) definitions
 */

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

struct elf_shdr
{
    uint32_t name;
    uint32_t type;
    uint64_t flags;
    uint64_t addr;
    uint64_t offset;
    uint64_t size;
    uint32_t link;
    uint32_t info;
    uint64_t addralign;
    uint64_t entsize;
};

// elf_shdr.type values
#define SHT_NULL	  0
#define SHT_PROGBITS  1
#define SHT_SYMTAB	  2
#define SHT_STRTAB	  3
#define SHT_RELA	  4
#define SHT_HASH	  5
#define SHT_DYNAMIC	  6
#define SHT_NOTE	  7
#define SHT_NOBITS	  8
#define SHT_REL		  9
#define SHT_SHLIB	  10
#define SHT_DYNSYM	  11

struct elf_sym
{
    uint32_t name;
    uint8_t  info;
    uint8_t  other;
    uint16_t shndx;
    uint64_t value;
    uint64_t size;
};

#define SHN_UNDEF      0
#define SHN_LORESERVE  0xff00
#define SHN_LOPROC     0xff00

#define PT_NULL 0
#define PT_LOAD 1

struct elf_rela
{
    uint64_t offset;
    uint64_t info;
    int64_t  addend; 
};

#define R_X86_64_64	    1	/* Direct 64 bit  */
#define R_X86_64_PC32   2	/* PC relative 32 bit signed */
#define R_X86_64_32		10	/* Direct 32 bit zero extended */

#define ELF64_R_SYM(i)  ((i) >> 32)
#define ELF64_R_TYPE(i) ((i) & 0xffffffff)