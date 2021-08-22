#pragma once

struct task
{
    unsigned int id;
    struct list threads; // struct thread*[]
};

struct task* task_creat(const char* file, char* argv[], char* envp[]);
struct task* task_kcreat(uintptr_t entry);
