#include <micro/module.h>
#include <micro/vfs.h>
#include <micro/elf.h>
#include <arch/mmu.h>

#define MODULE_BASE 0xffffffffa0000000

static uintptr_t map_module(size_t size)
{
    //static uintptr_t base = MODULE_BASE;

    if (size % PAGE4K != 0)
        size += PAGE4K - (size % PAGE4K);

    uintptr_t base = mmu_kalloc(size);
    for (uintptr_t i = base; i < base + size; i += PAGE4K)
    {
        mmu_kmap(i, mmu_alloc_phys(), PAGE_PR | PAGE_RW);
    }

    //uintptr_t ret = base;
    //base += size;
    return base;
}

struct elf_shdr* get_section(struct elf_hdr* hdr, int idx)
{
    return (uintptr_t)hdr + hdr->sh_off + hdr->sh_ent_size * idx;
}

void module_load(const char* path)
{
    struct file* file = vfs_resolve(path);

    // TODO: don't read unnecessary information
    void* data = map_module(file->size);
    vfs_read(file, data, 0, file->size);

    struct elf_hdr* header = (struct elf_hdr*)data;
    // TODO: verify signature

    struct elf_shdr* string_section = get_section(header, header->sh_str_idx);

    for (unsigned int i = 0; i < header->sh_num; i++)
    {
        struct elf_shdr* shdr = get_section(header, i);
        if (shdr->type != SHT_SYMTAB) continue;

        struct elf_sym* symtab = (struct elf_sym*)shdr->addr;

        for (unsigned int j = 0; j < shdr->size / sizeof(struct elf_sym); j++)
        {
            if (symtab[j].shndx != SHN_UNDEF) continue;
            char* name = (uintptr_t)data + ((char*)string_section->offset) + symtab[j].name;
            printk("Symbol: %x %s\n", symtab[j].shndx, name);
        }
    }

    
}