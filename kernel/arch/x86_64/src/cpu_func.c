#include <cpu_func.h>

uintptr_t rcr0()
{
    uintptr_t cr0;
    asm volatile ("mov %%cr0, %0" : "=r"(cr0));
    return cr0;
}

uintptr_t rcr2()
{
    uintptr_t cr2;
    asm volatile ("mov %%cr2, %0" : "=r"(cr2));
    return cr2;
}

uintptr_t rcr3()
{
    uintptr_t cr3;
    asm volatile ("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

uintptr_t rcr4()
{
    uintptr_t cr4;
    asm volatile ("mov %%cr4, %0" : "=r"(cr4));
    return cr4;
}