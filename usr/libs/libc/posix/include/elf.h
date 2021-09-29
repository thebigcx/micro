/*
 *  ELF (Executable and Linker Format) definitions
 */

#pragma once

#include <stdint.h>

typedef uint16_t   Elf64_Half;
typedef uint32_t   Elf64_Word;
typedef	int32_t    Elf64_Sword;
typedef uint64_t   Elf64_Addr;
typedef uint64_t   Elf64_Off;
typedef uint64_t   Elf64_Xword;
typedef	int64_t    Elf64_Sxword;
typedef uint16_t   Elf64_Section;
typedef Elf64_Half Elf64_Versym;

#define EI_NIDENT 16

typedef struct
{
    unsigned char e_ident[EI_NIDENT];	/* Magic number and other info */
    Elf64_Half	  e_type;			/* Object file type */
    Elf64_Half	  e_machine;		/* Architecture */
    Elf64_Word	  e_version;		/* Object file version */
    Elf64_Addr	  e_entry;		/* Entry point virtual address */
    Elf64_Off	  e_phoff;		/* Program header table file offset */
    Elf64_Off	  e_shoff;		/* Section header table file offset */
    Elf64_Word	  e_flags;		/* Processor-specific flags */
    Elf64_Half	  e_ehsize;		/* ELF header size in bytes */
    Elf64_Half	  e_phentsize;		/* Program header table entry size */
    Elf64_Half	  e_phnum;		/* Program header table entry count */
    Elf64_Half	  e_shentsize;		/* Section header table entry size */
    Elf64_Half	  e_shnum;		/* Section header table entry count */
    Elf64_Half	  e_shstrndx;		/* Section header string table index */
} Elf64_Ehdr;

#define EI_CLASS    4
#define ELFCLASS64  2 // 64-bit object

#define EI_DATA     5
#define ELFDATA2LSB 1 // 2's complement, little endian

#define EI_VERSION  6
#define EV_CURRENT  1

#define EM_X86_64 62

typedef struct
{
    Elf64_Word  p_type;		/* Segment type */
    Elf64_Word  p_flags;	/* Segment flags */
    Elf64_Off	p_offset;	/* Segment file offset */
    Elf64_Addr  p_vaddr;	/* Segment virtual address */
    Elf64_Addr  p_paddr;	/* Segment physical address */
    Elf64_Xword p_filesz;	/* Segment size in file */
    Elf64_Xword p_memsz;	/* Segment size in memory */
    Elf64_Xword p_align;	/* Segment alignment */
} Elf64_Phdr;

typedef struct
{
    Elf64_Word	sh_name;	  /* Section name (string tbl index) */
    Elf64_Word	sh_type;	  /* Section type */
    Elf64_Xword	sh_flags;	  /* Section flags */
    Elf64_Addr	sh_addr;	  /* Section virtual addr at execution */
    Elf64_Off	sh_offset;	  /* Section file offset */
    Elf64_Xword	sh_size;	  /* Section size in bytes */
    Elf64_Word	sh_link;	  /* Link to another section */
    Elf64_Word	sh_info;	  /* Additional section information */
    Elf64_Xword	sh_addralign; /* Section alignment */
    Elf64_Xword	sh_entsize;	  /* Entry size if section holds table */
} Elf64_Shdr;

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

typedef struct
{
    Elf64_Word	  st_name;		/* Symbol name (string tbl index) */
    unsigned char st_info;		/* Symbol type and binding */
    unsigned char st_other;		/* Symbol visibility */
    Elf64_Section st_shndx;		/* Section index */
    Elf64_Addr	  st_value;		/* Symbol value */
    Elf64_Xword	  st_size;		/* Symbol size */
} Elf64_Sym;

#define SHN_UNDEF      0
#define SHN_LORESERVE  0xff00
#define SHN_LOPROC     0xff00

#define PT_NULL    0
#define PT_LOAD    1
#define PT_DYNAMIC 2
#define PT_INTERP  3

#define PF_X       (1 << 0)
#define PF_W       (1 << 1)
#define PF_R       (1 << 2)

typedef struct
{
    Elf64_Addr	 r_offset;		/* Address */
    Elf64_Xword	 r_info;		/* Relocation type and symbol index */
    Elf64_Sxword r_addend;		/* Addend */
} Elf64_Rela;

#define R_X86_64_64	    1	/* Direct 64 bit  */
#define R_X86_64_PC32   2	/* PC relative 32 bit signed */
#define R_X86_64_32		10	/* Direct 32 bit zero extended */

#define ELF64_R_SYM(i)  ((i) >> 32)
#define ELF64_R_TYPE(i) ((i) & 0xffffffff)

#define ELFMAG0  0x7f
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'

typedef struct
{
    Elf64_Sxword d_tag;			/* Dynamic entry type */
    union
    {
        Elf64_Xword d_val;		/* Integer value */
        Elf64_Addr  d_ptr;		/* Address value */
    } d_un;
} Elf64_Dyn;

#define DT_NULL		0 /* Marks end of dynamic section */
#define DT_NEEDED	1 /* Name of needed library */