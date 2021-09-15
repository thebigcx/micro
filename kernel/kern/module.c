/*
 *  Module Loader - takes a relocatable object
 *  file (.ko) and links it to the kernel
 */

#include <micro/module.h>
#include <micro/elf.h>
#include <arch/mmu.h>
#include <micro/errno.h>
#include <micro/ksym.h>

#define MOD_MAX 64
static struct module modules[MOD_MAX];

static struct elf_shdr* get_section(struct elf_hdr* hdr, int idx)
{
    return (struct elf_shdr*)((uintptr_t)hdr + hdr->sh_off + hdr->sh_ent_size * idx);
}

static int find_module(const char* name)
{
    for (unsigned int i = 0; i < MOD_MAX; i++)
    {
        if (modules[i].meta && !strcmp(name, modules[i].meta->name))
            return i;
    }

    return -1;
}

int module_load(void* data, size_t len)
{
    uintptr_t base = mmu_map_module(len);
    memcpy((void*)base, data, len);

    struct elf_hdr* header = (struct elf_hdr*)base;
    // TODO: verify signature

    for (unsigned int i = 0; i < header->sh_num; i++)
    {
        struct elf_shdr* shdr = get_section(header, i);
        if (shdr->type == SHT_NOBITS)
        {
            shdr->addr = mmu_map_module(shdr->size);
            memset(shdr->addr, 0, shdr->size);
        }
        else
            shdr->addr = base + shdr->offset;
    }

    struct modmeta* meta = NULL;

    /* Load the symbol tables - if a symbol is undefined, bind it
       to a kernel symbol, else set it to the position in
       the loaded module. */

    for (unsigned int i = 0; i < header->sh_num; i++)
    {
        struct elf_shdr* shdr = get_section(header, i);
        if (shdr->type != SHT_SYMTAB) continue;

        struct elf_shdr* strsect = get_section(header, shdr->link);
        struct elf_sym* symtab = (struct elf_sym*)shdr->addr;

        for (unsigned int j = 0; j < shdr->size / sizeof(struct elf_sym); j++)
        {
            char* name = strsect->addr + symtab[j].name;

            if (symtab[j].shndx == SHN_UNDEF)
                symtab[j].value = ksym_lookup(name);
            else if (symtab[j].shndx > 0 && symtab[j].shndx < SHN_LOPROC)
            {
                struct elf_shdr* symbol_hdr = get_section(header, symtab[j].shndx);
                symtab[j].value = symbol_hdr->addr + symtab[j].value;
            }

            // Module metadata defining init() and fini() among other things
            if (!strcmp(name, "meta"))
                meta = symtab[j].value;
        }
    }

    for (unsigned int i = 0; i < header->sh_num; i++)
    {
        struct elf_shdr* shdr = get_section(header, i);
        if (shdr->type != SHT_RELA) continue;

        struct elf_rela* reltab = (struct elf_rela*)shdr->addr;
        struct elf_shdr* targsect = get_section(header, shdr->info);

        struct elf_shdr* symsect = get_section(header, shdr->link);
        struct elf_sym* symtab = (struct elf_sym*)symsect->addr;

        for (unsigned int j = 0; j < shdr->size / shdr->entsize; j++)
        {
            uintptr_t targ = targsect->addr + reltab[j].offset;

            switch (ELF64_R_TYPE(reltab[j].info))
            {
                case R_X86_64_64:
                    *((uint64_t*)targ) = symtab[ELF64_R_SYM(reltab[j].info)].value + reltab[j].addend;
                    break;
                
                case R_X86_64_32:
                    *((uint32_t*)targ) = symtab[ELF64_R_SYM(reltab[j].info)].value + reltab[j].addend;
                    break;

                case R_X86_64_PC32:
                    *((uint32_t*)targ) = symtab[ELF64_R_SYM(reltab[j].info)].value + reltab[j].addend - targ;
                    break;
            }
        }
    }

    if (find_module(meta->name) != -1)
    {
        mmu_unmap_module(base, len);
        return -EEXIST;
    }

    for (unsigned int i = 0; i < MOD_MAX; i++)
    {
        if (!modules[i].addr)
        {
            modules[i] = (struct module)
            {
                .addr = base,
                .size = len,
                .meta = meta
            };

            meta->init();
            return 0;
        }
    }

    return -ENOMEM;
}

int module_free(const char* name)
{
    int i = find_module(name);
    if (i == -1) return -ENOENT;

    modules[i].meta->fini();

    mmu_unmap_module(modules[i].addr, modules[i].size);
    memset(&modules[i], 0, sizeof(struct module));
    
    return 0;
}

void modules_init()
{
    memset(modules, 0, sizeof(modules));
}