#pragma once

#include <types.h>
#include <descs.h>
#include <list.h>
#include <lock.h>

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

#define rdmsr(msr, l, h) asm volatile ("rdmsr" : "=a"(l), "=d"(h) : "c"(msr))
#define wrmsr(msr, l, h) asm volatile ("wrmsr" :: "a"(l), "d"(h), "c"(msr))
#define cli() asm volatile ("cli")
#define sti() asm volatile ("sti")

struct cpu_info* cpu_curr();

void cpu_set_kstack(struct cpu_info* cpu, uintptr_t kstack);

void arch_init_thread(struct thread* thread, int usr);
void arch_switch_ctx(struct thread* thread);
