#include <micro/binfmt.h>
#include <micro/elf.h>
#include <arch/mmu.h>
#include <micro/task.h>
#include <micro/stdlib.h>
#include <arch/cpu_func.h>
#include <micro/thread.h>
#include <micro/vfs.h>

#define PUSH_STR(stack, x) { stack -= strlen(x) + 1; strcpy((char*)stack, x); }
#define PUSH(stack, type, x) { stack -= sizeof(type); *((type*)stack) = x; }

static void setup_user_stack(struct task* task, const char* argv[], const char* envp[])
{
    int argc = 0, envc = 0;
    uintptr_t args[64], envs[64];

    uintptr_t cr3 = rcr3();

    lcr3(task->vm_map->pml4_phys);

    // Push the raw strings onto the stack
    while (argv[argc])
    {
        PUSH_STR(task->main->regs.rsp, argv[argc]);
        args[argc] = task->main->regs.rsp;
        argc++;
    }

    while (envp[envc])
    {
        PUSH_STR(task->main->regs.rsp, envp[envc]);
        envs[envc] = task->main->regs.rsp;
        envc++;
    }

    // Pointer-align the stack for char* argv[]
    task->main->regs.rsp -= (task->main->regs.rsp % 8);

    // Null-terminate the argv[] array (reverse-order)
    PUSH(task->main->regs.rsp, uintptr_t, (uintptr_t)NULL);

    // Push pointers in reverse order
    for (int i = argc - 1; i >= 0; i--)
        PUSH(task->main->regs.rsp, uintptr_t, args[i]);

    uintptr_t argv_ptr = task->main->regs.rsp;

    // Null-terminate the envp[] array (reverse-order)
    PUSH(task->main->regs.rsp, uintptr_t, (uintptr_t)NULL);

    for (int i = envc - 1; i >= 0; i--)
        PUSH(task->main->regs.rsp, uintptr_t, envs[i]);

    uintptr_t envp_ptr = task->main->regs.rsp;

    lcr3(cr3);

    task->main->regs.rdi = argc;
    task->main->regs.rsi = argv_ptr;
    task->main->regs.rdx = envp_ptr;
}

uintptr_t elf_load(struct task* task, void* data,
                   const char* argv[], const char* envp[])
{
    struct elf_hdr* header = (struct elf_hdr*)data;
    
    //if (!valid_elf(*header)) // TODO: return -ELIBBAD
        //return -1;

    // Check if need to load a dynamic linker
    for (unsigned int i = 0; i < header->ph_num; i++)
    {
        struct elf_phdr* phdr = (struct elf_phdr*)((uintptr_t)data + header->ph_off + header->ph_ent_size * i);

        if (phdr->type == PT_INTERP)
        {
            size_t argc = 1;
            while (argv[argc - 1]) argc++;

            char* nargv[argc + 1];
            nargv[0] = "/lib/ld.so";
            memcpy(&nargv[1], &argv[0], argc * sizeof(const char*));

            // TODO: this leaks memory
            struct file* interp = kmalloc(sizeof(struct file));
            int e = vfs_resolve(nargv[0], interp);

            if (e) return e;

            void* data = kmalloc(interp->size);
            vfs_read(interp, data, 0, interp->size);

            return elf_load(task, data, (const char**)nargv, envp);
        }
    }

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
                // TODO: use the PHDR flags to determine page flags
                mmu_map(task->vm_map, i, mmu_alloc_phys(), PAGE_PR | PAGE_RW | PAGE_USR);
            }

            uintptr_t cr3 = rcr3();
            
            lcr3(task->vm_map->pml4_phys);
            memset((void*)begin, 0, memsize);
            memcpy((void*)begin, (void*)((uintptr_t)data + phdr->offset), filesize);
            lcr3(cr3);
        }
    }

    setup_user_stack(task, argv, envp);

    return header->entry;
}