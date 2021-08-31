#include <smp.h>
#include <types.h>
#include <mmu.h>
#include <debug/syslog.h>
#include <lapic.h>
#include <cpu.h>
#include <stdlib.h>
#include <timer.h>
#include <list.h>
#include <cpu_func.h>

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
    g_cpu_cnt++;

    struct cpu_info* cpu = &g_cpus[id];
    cpu->ready = list_create();
    cpu->threads = list_create();
    cpu->current = NULL;
    cpu->lock = 0;

    gdt_init_cpu(cpu);
    idt_init_cpu(cpu);
    
    lapic_enable(); 

    _ap_done = 1;

    printk("%d", id);

    sti();
    for (;;);
}

static void init_cpu(uint16_t id)
{
    uintptr_t cr3 = rcr3();
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

    timer_wait(10000000);

    if (!_ap_done)
    {
        printk("error: cannot start cpu");
    }
}

void smp_init(struct list* ids)
{
    printk("starting cpus: %d", ids->size);

    list_foreach(ids)
    {
        init_cpu(*((uint16_t*)node->data));
    }
}
