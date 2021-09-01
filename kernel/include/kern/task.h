#pragma once

#include <mmu.h>
#include <list.h>

struct task
{
    unsigned int id;

    struct list threads;  // struct thread*[]
    struct list fds;      // struct fd*[]
    struct list children; // struct task*[]
    struct task* parent;

    struct vm_map* vm_map;
};

struct thread;

struct task* task_idle();
struct task* task_creat(struct task* parent, const void* buffer, char* argv[], char* envp[]);
struct task* task_kcreat(struct task* parent, uintptr_t entry);
struct task* task_clone(const struct task* src, struct thread* calling);
struct task* task_curr();