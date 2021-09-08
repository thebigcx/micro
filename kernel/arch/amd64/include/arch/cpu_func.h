#pragma once

#include <micro/types.h>
#include <micro/platform.h>
#include <arch/descs.h>

FORCE_INLINE void lidt(volatile struct descptr* idtr)
{
    asm volatile ("lidt (%0)" :: "r"(idtr));
}

FORCE_INLINE void ltr(uint16_t seg)
{
    asm volatile ("ltr %0" :: "r"(seg));
}

FORCE_INLINE void lgdt(struct descptr* gdtr)
{
    asm volatile ("lgdt (%0)" :: "r"(gdtr));
}

// Loads the GDT and reloads the stale segment registers
extern void lgdt_full(struct descptr*, uint16_t, uint16_t);

FORCE_INLINE void cli()
{
    asm volatile ("cli");
}

FORCE_INLINE void sti()
{
    asm volatile ("sti");
}

FORCE_INLINE void invlpg(uintptr_t virt)
{
    asm volatile ("invlpg (%0)" :: "r"(virt) : "memory");
}

FORCE_INLINE uintptr_t rdmsr(unsigned int msr)
{
    uint32_t lo, hi;
    asm volatile ("rdmsr" : "=d"(hi), "=a"(lo) : "c"(msr));
    return ((uintptr_t)hi << 32) | (uintptr_t)lo;
}

FORCE_INLINE void wrmsr(unsigned int msr, uintptr_t val)
{
    asm volatile ("wrmsr" :: "a"(val & 0xffffffff), "d"(val >> 32), "c"(msr));
}

FORCE_INLINE uintptr_t rcr0()
{
    uintptr_t cr0;
    asm volatile ("mov %%cr0, %0" : "=r"(cr0));
    return cr0;
}

FORCE_INLINE uintptr_t rcr2()
{
    uintptr_t cr2;
    asm volatile ("mov %%cr2, %0" : "=r"(cr2));
    return cr2;
}

FORCE_INLINE uintptr_t rcr3()
{
    uintptr_t cr3;
    asm volatile ("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

FORCE_INLINE uintptr_t rcr4()
{
    uintptr_t cr4;
    asm volatile ("mov %%cr4, %0" : "=r"(cr4));
    return cr4;
}

FORCE_INLINE void lcr0(uintptr_t cr0)
{
    asm volatile ("mov %0, %%cr0" :: "r"(cr0));
}

FORCE_INLINE void lcr2(uintptr_t cr2)
{
    asm volatile ("mov %0, %%cr2" :: "r"(cr2));
}

FORCE_INLINE void lcr3(uintptr_t cr3)
{
    asm volatile ("mov %0, %%cr3" :: "r"(cr3));
}

FORCE_INLINE void lcr4(uintptr_t cr4)
{
    asm volatile ("mov %0, %%cr4" :: "r"(cr4));
}

FORCE_INLINE void pause()
{
    asm volatile ("pause");
}

// TODO: implemenent wrapper over CPUID