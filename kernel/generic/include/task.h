#pragma once

#include <mmu.h>
#include <list.h>

struct task
{
    unsigned int id;
    struct list threads; // struct thread*[]
    struct vm_map* vm_map;
};

struct task* task_idle();
struct task* task_creat(const void* buffer, char* argv[], char* envp[]);
struct task* task_kcreat(uintptr_t entry);
