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
            uintptr_t size = phdr->memsz;

            if (begin % PAGE4K)
            {
                begin -= begin % PAGE4K;
                size += begin % PAGE4K;
            }

            if (size % PAGE4K)
                size = size - (size % PAGE4K) + PAGE4K;

            for (uintptr_t i = begin; i < begin + size; i += PAGE4K)
            {
                mmu_map(task->vm_map, i, mmu_alloc_phys(), PAGE_PR | PAGE_RW | PAGE_USR);
            }
        
            // TODO: better way of doing this
            uintptr_t cr3 = rcr3();
            
            lcr3(task->vm_map->pml4_phys);
            memcpy((void*)begin, (void*)((uintptr_t)data + phdr->offset), size);
            lcr3(cr3);
        }
    }

    return header->entry;
}