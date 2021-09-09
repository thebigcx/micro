#include <micro/binfmt.h>
#include <micro/elf.h>
#include <arch/mmu.h>
#include <micro/task.h>
#include <micro/stdlib.h>
#include <arch/cpu_func.h>

uintptr_t elf_load(struct task* task, void* data)
{
    struct elf_hdr* header = (struct elf_hdr*)data;
    
    //if (!valid_elf(*header))
        //return -1;

    for (unsigned int i = 0; i < header->ph_num; i++)
    {
        struct elf_phdr* phdr = (struct elf_phdr*)((uintptr_t)data + header->ph_off + header->ph_ent_size * i);

        if (phdr->type == PT_LOAD)
        {
            uintptr_t begin = phdr->vaddr;
            uintptr_t memsize = phdr->memsz;
            uintptr_t filesize = phdr->filesz;

            uintptr_t page_begin = begin - (begin % PAGE4K);
            uintptr_t page_cnt = memsize - (memsize % PAGE4K) + PAGE4K * 2;

            for (uintptr_t i = page_begin; i < page_begin + page_cnt; i += PAGE4K)
            {
                mmu_map(task->vm_map, i, mmu_alloc_phys(), PAGE_PR | PAGE_RW | PAGE_USR);
            }
        
            // TODO: better way of doing this
            uintptr_t cr3 = rcr3();
            
            lcr3(task->vm_map->pml4_phys);
            memset((void*)begin, 0, memsize);
            memcpy((void*)begin, (void*)((uintptr_t)data + phdr->offset), filesize);
            lcr3(cr3);
        }
    }

    return header->entry;
}