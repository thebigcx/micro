#pragma once

#include <cpu.h>

#define THREAD_RUNNING  0
#define THREAD_READY    1
#define THREAD_DEAD     2
#define THREAD_WAIT     3

struct task;

struct thread
{
    unsigned int id;
    int state;
    struct regs regs;
    struct task* parent;
    uintptr_t kstack;
};

struct thread* thread_creat(struct task* parent, uintptr_t entry, int usr);
