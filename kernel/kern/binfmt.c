#include <micro/binfmt.h>
#include <micro/elf.h>
#include <arch/mmu.h>
#include <micro/task.h>
#include <micro/stdlib.h>
#include <arch/cpu_func.h>
#include <micro/thread.h>
#include <micro/vfs.h>
#include <micro/errno.h>
#include <micro/vmmap.h>

#define PUSH_STR(stack, x) { stack -= strlen(x) + 1; strcpy((char*)stack, x); }
#define PUSH(stack, type, x) { stack -= sizeof(type); *((type*)stack) = x; }

void gen_auxv(auxv_t* aux, struct task* task)
{
    size_t i = 0;
    
    aux[i++] = (auxv_t) { .a_type = AT_PAGESZ,
        .a_un.a_val = 0x1000 };
    aux[i++] = (auxv_t) { .a_type = AT_UID,
        .a_un.a_val = task->ruid };
    aux[i++] = (auxv_t) { .a_type = AT_EUID,
        .a_un.a_val = task->euid };
    aux[i++] = (auxv_t) { .a_type = AT_GID,
        .a_un.a_val = task->rgid };
    aux[i++] = (auxv_t) { .a_type = AT_EGID,
        .a_un.a_val = task->egid };
    aux[i++] = (auxv_t) { .a_type = AT_SECURE,
        .a_un.a_val = task->euid != task->ruid || task->egid != task->rgid };
    aux[i++] = (auxv_t) { .a_type = AT_NULL };
}

void setup_user_stack(struct task* task, char* const argv[], char* const envp[])
{
    int argc = 0, envc = 0;
    uintptr_t args[64], envs[64];

    auxv_t auxv[64];
    gen_auxv(auxv, task);

    uintptr_t cr3 = rcr3();

    vm_set_curr(task->vm_map);

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

    // Auxiliary vector
    size_t auxv_len = 0;
    while (auxv[auxv_len].a_type != AT_NULL) auxv_len++;

    for (int i = auxv_len; i >= 0; i--)
        PUSH(task->main->regs.rsp, auxv_t, auxv[i]);

    // Null-terminate the envp[] array (reverse-order)
    PUSH(task->main->regs.rsp, uintptr_t, (uintptr_t)NULL);

    for (int i = envc - 1; i >= 0; i--)
        PUSH(task->main->regs.rsp, uintptr_t, envs[i]);

    // Null-terminate the argv[] array (reverse-order)
    PUSH(task->main->regs.rsp, uintptr_t, (uintptr_t)NULL);

    // Push pointers in reverse order
    for (int i = argc - 1; i >= 0; i--)
        PUSH(task->main->regs.rsp, uintptr_t, args[i]);

    PUSH(task->main->regs.rsp, uintptr_t, argc);

    task->main->regs.rdi = task->main->regs.rsp;

    lcr3(cr3);
}

// TODO: this is very architecture-dependent
int elf_valid(void* data)
{
    struct elf_hdr* header = data;
    return header->ident[0]          == ELFMAG0
        && header->ident[1]          == ELFMAG1
        && header->ident[2]          == ELFMAG2
        && header->ident[3]          == ELFMAG3
        && header->ident[EI_CLASS]   == ELFCLASS64
        && header->ident[EI_DATA]    == ELFDATA2LSB
        && header->machine           == EM_X86_64
        && header->ident[EI_VERSION] == EV_CURRENT;
}

int elf_load(struct vm_map* vm_map, void* data, char* const argv[],
             char* const envp[], struct elfinf* inf)
{
    inf->brk = 0;

    struct elf_hdr* header = (struct elf_hdr*)data;

    for (unsigned int i = 0; i < header->ph_num; i++)
    {
        struct elf_phdr* phdr = (struct elf_phdr*)((uintptr_t)data + header->ph_off + header->ph_ent_size * i);

        if (phdr->type == PT_LOAD)
        {
            uintptr_t begin = phdr->vaddr;
            uintptr_t memsize = phdr->memsz;
            uintptr_t filesize = phdr->filesz;

            // TODO: this isn't right
            uintptr_t page_begin = begin - (begin % PAGE4K);
            uintptr_t page_cnt = memsize - (memsize % PAGE4K) + PAGE4K * 2;

            vm_do_mmap(vm_map, page_begin, page_cnt, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

            uintptr_t cr3 = rcr3();

            vm_set_curr(vm_map);            
            memset((void*)begin, 0, memsize);
            memcpy((void*)begin, (void*)((uintptr_t)data + phdr->offset), filesize);
            lcr3(cr3);
        
            if (inf->brk < page_begin + page_cnt)
                inf->brk = page_begin + page_cnt;
        }
    }

    inf->entry = header->entry;
    return 0;
}

// Check for a program interpreter - returns NULL if none found
const char* elf_getinterp(void* data)
{
    struct elf_hdr* header = (struct elf_hdr*)data;

    for (unsigned int i = 0; i < header->ph_num; i++)
    {
        struct elf_phdr* phdr = (struct elf_phdr*)((uintptr_t)data + header->ph_off + header->ph_ent_size * i);

        if (phdr->type == PT_INTERP)
        {
            // TODO: get actual interpreter path
            return "/lib/ld.so";
        }
    }

    return NULL;
}
