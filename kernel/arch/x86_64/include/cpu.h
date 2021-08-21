#pragma once

#include <types.h>
#include <descs.h>

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

struct cpu_info
{
    struct tss tss;
    union gdtent gdt[7];
};

#define rdmsr(msr, l, h) asm volatile ("rdmsr" : "=a"(l), "=d"(h) : "c"(msr))
#define wrmsr(msr, l, h) asm volatile ("wrmsr" :: "a"(l), "d"(h), "c"(msr))

void eoi();
