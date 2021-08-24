#include <cpu.h>

struct cpu_info g_cpus[MAX_CPUS];

void outb(uint16_t port, uint8_t val)
{
    asm volatile ("outb %0, %1" :: "a"(val), "Nd"(port));
}

void outw(uint16_t port, uint16_t val)
{
    asm volatile ("outw %0, %1" :: "a"(val), "Nd"(port));
}

void outl(uint16_t port, uint32_t val)
{
    asm volatile ("outl %0, %1" :: "a"(val), "Nd"(port));
}

uint8_t inb(uint16_t port)
{
    uint8_t v;
    asm volatile ("inb %1, %0" : "=a"(v) : "Nd"(port));
    return v;
}

uint16_t inw(uint16_t port)
{
    uint16_t v;
    asm volatile ("inw %1, %0" : "=a"(v) : "Nd"(port));
    return v;
}

uint32_t inl(uint16_t port)
{
    uint32_t v;
    asm volatile ("inl %1, %0" : "=a"(v) : "Nd"(port));
    return v;
}

uintptr_t read_cr0()
{
    uintptr_t cr0;
    asm volatile ("mov %%cr0, %0" : "=r"(cr0));
    return cr0;
}

uintptr_t read_cr2()
{
    uintptr_t cr2;
    asm volatile ("mov %%cr2, %0" : "=r"(cr2));
    return cr2;
}

uintptr_t read_cr3()
{
    uintptr_t cr3;
    asm volatile ("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

uintptr_t read_cr4()
{
    uintptr_t cr4;
    asm volatile ("mov %%cr4, %0" : "=r"(cr4));
    return cr4;
}

struct cpu_info* cpu_curr()
{
    uintptr_t id;
    asm volatile ("cpuid" : "=b"(id) : "a"(1));
    return &g_cpus[id >> 24];
}

void arch_switch_ctx(struct thread* thread)
{
    // TODO
}
