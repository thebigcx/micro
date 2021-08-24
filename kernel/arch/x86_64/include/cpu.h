#pragma once

#include <types.h>
#include <descs.h>
#include <list.h>
#include <lock.h>

struct regs
{
    uintptr_t r15;
    uintptr_t r14;
    uintptr_t r13;
    uintptr_t r12;
    uintptr_t r11;
    uintptr_t r10;
    uintptr_t r9;
    uintptr_t r8;
    uintptr_t rbp;
    uintptr_t rdi;
    uintptr_t rsi;
    uintptr_t rdx;
    uintptr_t rcx;
    uintptr_t rbx;
    uintptr_t rax;
    uintptr_t rip;
    uintptr_t cs;
    uintptr_t rflags;
    uintptr_t rsp;
    uintptr_t ss;
};

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

void outb(uint16_t port, uint8_t val);
void outw(uint16_t port, uint16_t val);
void outl(uint16_t port, uint32_t val);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);
uint32_t inl(uint16_t port);
uintptr_t read_cr0();
uintptr_t read_cr2();
uintptr_t read_cr3();
uintptr_t read_cr4();

struct cpu_info* cpu_curr();

void cpu_set_kstack(struct cpu_info* cpu, uintptr_t kstack);

void arch_init_thread(struct thread* thread, int usr);
void arch_switch_ctx(struct thread* thread);
