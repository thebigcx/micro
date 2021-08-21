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
#define cli() asm volatile ("cli")
#define sti() asm volatile ("sti")

inline void outb(uint16_t port, uint8_t val)
{
    asm volatile ("outb %0, %1" :: "a"(val), "Nd"(port));
}

inline void outw(uint16_t port, uint16_t val)
{
    asm volatile ("outw %0, %1" :: "a"(val), "Nd"(port));
}

inline void outl(uint16_t port, uint32_t val)
{
    asm volatile ("outl %0, %1" :: "a"(val), "Nd"(port));
}

inline uint8_t inb(uint16_t port)
{
    uint8_t v;
    asm volatile ("inb %1, %0" : "=a"(v) : "Nd"(port));
    return v;
}

inline uint16_t inw(uint16_t port)
{
    uint16_t v;
    asm volatile ("inw %1, %0" : "=a"(v) : "Nd"(port));
    return v;
}

inline uint32_t inl(uint16_t port)
{
    uint32_t v;
    asm volatile ("inl %1, %0" : "=a"(v) : "Nd"(port));
    return v;
}

void eoi();
