#pragma once

#include <reg.h>

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

    struct regs syscall_regs;
};

void thread_start(struct thread* thread);
struct thread* thread_creat(struct task* parent, uintptr_t entry, int usr);
struct thread* thread_clone(struct task* parent, struct thread* src);
struct thread* thread_curr();