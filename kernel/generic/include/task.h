#pragma once

#include <mmu.h>
#include <list.h>

struct task
{
    unsigned int id;
    struct list threads; // struct thread*[]
    struct pagedir pd;
};

struct task* task_idle();
struct task* task_creat(const void* buffer, char* argv[], char* envp[]);
struct task* task_kcreat(uintptr_t entry);
