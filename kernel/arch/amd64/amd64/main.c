#include <boot.h>
#include <cpu.h>
#include <mmu.h>
#include <acpi.h>
#include <lapic.h>
#include <smp.h>
#include <heap.h>
#include <cpu_func.h>
#include <vfs.h>
#include <stdlib.h>
#include <task.h>
#include <sched.h>
#include <init.h>

void main(struct bootparams params)
{
    struct genbootparams genparams;

    mmu_alloc_phys_at(0, 0x100);
    mmu_init();

    printk("initrd: %x - %x\n", params.initrd_phys_start, params.initrd_phys_end);
    uintptr_t size = (params.initrd_phys_end - params.initrd_phys_start + PAGE4K) / PAGE4K;
    uintptr_t vaddr = mmu_kalloc(size);
    for (uintptr_t i = 0; i < size * PAGE4K; i += PAGE4K)
        mmu_kmap(vaddr + i, params.initrd_phys_start + i, PAGE_PR | PAGE_RW);

    genparams.initrd_start = vaddr;
    genparams.initrd_end = vaddr + size * PAGE4K;

    gdt_init_cpu(&g_cpus[0]);
    idt_init();
    idt_init_cpu(&g_cpus[0]);
    printk("loaded gdt\n");
    
    heap_init();
    printk("initialized heap\n");

    acpi_init(params.rsdp);
    //acpi_parse_madt();
    printk("initialized ACPI\n");

    lapic_setup();
    printk("initialized LAPIC\n");

    ioapic_init();
    printk("initalized IOAPIC\n");

    timer_init();
    printk("initialized timer\n");

    sti();
    
    smp_init();
    printk("initialized other CPUs\n");

    generic_init(genparams);
}