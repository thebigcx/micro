#include <smp.h>
#include <types.h>
#include <mmu.h>
#include <debug/syslog.h>
#include <lapic.h>
#include <cpu.h>
#include <stdlib.h>
#include <timer.h>

#define TRMP_ENTRY 0x8000
#define AP_CR3     0x1000
#define AP_ENT     0x1008
#define AP_STACK   0x1010

// Set after CPU is done with trampoline
static volatile int _ap_done = 0;

// Defined in ap_start.S
extern void* _ap_bs_start;
extern void* _ap_bs_end;

static void ap_entry(uint16_t id)
{
    dbgln("another cpu");
    _ap_done = 1;

    for (;;);
}

static void init_cpu(uint16_t id)
{
    uintptr_t cr3 = read_cr3();
    uintptr_t stack = mmu_kalloc(1);
    mmu_kmap(stack, mmu_alloc_phys(), PAGE_PR | PAGE_RW);
    
    *((volatile uintptr_t*)AP_ENT)   = (uintptr_t)ap_entry;
    *((volatile uintptr_t*)AP_STACK) = stack + PAGE4K;
    *((volatile uintptr_t*)AP_CR3)   = cr3;

    memcpy((void*)TRMP_ENTRY, &_ap_bs_start, PAGE4K);

    _ap_done = 0;

    lapic_send_ipi(id, 0, DELIV_INIT);

    timer_wait(10000000);

    lapic_send_ipi(id, TRMP_ENTRY >> 12, DELIV_STRT | LVL_ASSRT);

    timer_wait(200000);
}

void smp_init()
{
    dbgln("starting cpus");
    init_cpu(1);
}
