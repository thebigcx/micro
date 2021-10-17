#pragma once

#include <arch/reg.h>
#include <micro/signal.h>
#include <micro/list.h>

#define THREAD_RUNNING  0
#define THREAD_READY    1
#define THREAD_DEAD     2
#define THREAD_WAITING  3
#define THREAD_BLOCKED  4

struct task;

struct thread
{
    pid_t id;
    int state;
    struct regs regs;
    struct task* parent;
    uintptr_t kstack;

    struct regs syscall_regs;
    
    struct list sigqueue;
    uint32_t    sigmask;
    uintptr_t   sigstack;
};

struct thread* thread_creat(struct task* parent, uintptr_t entry, int usr);
struct thread* thread_clone(struct task* parent, struct thread* src);
struct thread* thread_curr();
void thread_handle_signals(struct thread* thread);
void thread_block();
void thread_unblock(struct thread* thread);
void thread_free(struct thread* thread);

void thread_handle_contsig(struct thread* thread);
