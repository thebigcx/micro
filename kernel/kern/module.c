/*
 *  The Module Loader - takes a relocatable object
 *  file (.ko) and links it to the kernel
 */

#include <micro/module.h>
#include <micro/vfs.h>
#include <micro/elf.h>
#include <arch/mmu.h>

static struct elf_shdr* get_section(struct elf_hdr* hdr, int idx)
{
    return (struct elf_shdr*)((uintptr_t)hdr + hdr->sh_off + hdr->sh_ent_size * idx);
}

void module_load(const char* path)
{
    struct file* file = vfs_resolve(path);

    // Load the whole module into memory
    void* data = (void*)mmu_map_module(file->size);
    vfs_read(file, data, 0, file->size);

    struct elf_hdr* header = (struct elf_hdr*)data;
    // TODO: verify signature

    struct modmeta* meta = NULL;

    /* Load the symbol tables - if a symbol is undefined, bind it
       to a kernel symbol, else set it to the position in
       the loaded module. */

    for (unsigned int i = 0; i < header->sh_num; i++)
    {
        struct elf_shdr* shdr = get_section(header, i);
        if (shdr->type != SHT_SYMTAB) continue;

        struct elf_shdr* strsect = get_section(header, shdr->link);
        struct elf_sym* symtab = (struct elf_sym*)((uintptr_t)data + shdr->offset);

        for (unsigned int j = 0; j < shdr->size / sizeof(struct elf_sym); j++)
        {
            char* name = (uintptr_t)data + ((char*)strsect->offset) + symtab[j].name;

            if (symtab[j].shndx == SHN_UNDEF)
                symtab[j].value = ksym_lookup(name);
            else if (symtab[j].shndx > 0 && symtab[j].shndx < SHN_LOPROC)
            {
                struct elf_shdr* symbol_hdr = get_section(header, symtab[j].shndx);
                symtab[j].value = (uintptr_t)data + symtab[j].value + symbol_hdr->offset;
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

        struct elf_rela* reltab = (struct elf_rela*)((uintptr_t)data + shdr->offset);
        struct elf_shdr* targsect = get_section(header, shdr->info);

        struct elf_shdr* symsect = get_section(header, shdr->link);
        struct elf_sym* symtab = (struct elf_sym*)((uintptr_t)data + symsect->offset);

        for (unsigned int j = 0; j < shdr->size / shdr->entsize; j++)
        {
            uintptr_t targ = reltab[j].offset + ((uintptr_t)data + targsect->offset);
            printk("relocate: %d\n", ELF64_R_TYPE(reltab[j].info));

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

    printk("meta: %x\n", meta);
    printk("init: %x\nfini: %x\n", meta->init, meta->fini);

    meta->init();
}