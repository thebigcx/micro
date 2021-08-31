#pragma once

#include <types.h>
#include <descs.h>
#include <list.h>
#include <lock.h>
#include <reg.h>

struct thread;

struct cpu_info
{
    struct tss tss;
    union gdtent gdt[7];
    struct list threads;
    struct list ready;
    struct thread* current;
    struct thread* idle;
    lock_t lock;
};

#define MAX_CPUS 16

extern struct cpu_info g_cpus[MAX_CPUS];
extern unsigned int g_cpu_cnt;

struct cpu_info* cpu_curr();

void cpu_set_kstack(struct cpu_info* cpu, uintptr_t kstack);

void arch_init_thread(struct thread* thread, int usr);
void arch_switch_ctx(struct thread* thread);

FORCE_INLINE void arch_syscall_ret(struct regs* r, uintptr_t n)
{
    r->rax = n;
}

FORCE_INLINE uintptr_t arch_syscall_num(struct regs* r)
{
    return r->rax;
}

#define ARCH_SCARG0(r) (r->rdi)
#define ARCH_SCARG1(r) (r->rsi)
#define ARCH_SCARG2(r) (r->rdx)
#define ARCH_SCARG3(r) (r->r10)
#define ARCH_SCARG4(r) (r->r8)
#define ARCH_SCARG5(r) (r->r9)