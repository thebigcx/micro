#include <arch/boot.h>
#include <arch/cpu.h>
#include <arch/mmu.h>
#include <micro/acpi.h>
#include <arch/lapic.h>
#include <arch/smp.h>
#include <micro/heap.h>
#include <arch/cpu_func.h>
#include <micro/vfs.h>
#include <micro/stdlib.h>
#include <micro/task.h>
#include <micro/sched.h>
#include <micro/init.h>
#include <micro/fbdev.h>
#include <micro/debug.h>
#include <arch/ioapic.h>
#include <arch/timer.h>
#include <arch/fpu.h>
#include <micro/vga.h>

void main(struct bootparams params)
{
    struct genbootparams genparams;

    printk("initrd: %x - %x\n", params.initrd_phys_start, params.initrd_phys_end);
    printk("framebuffer: %dx%d with %d-bit color\n", params.fbwidth, params.fbheight, params.fbbpp);

    printk("initializing the mmu...");
    mmu_alloc_phys_at(0, 0x100);
    mmu_init();

    uintptr_t size = (params.initrd_phys_end - params.initrd_phys_start + PAGE4K) / PAGE4K;
    uintptr_t vaddr = mmu_kalloc(size);
    for (uintptr_t i = 0; i < size * PAGE4K; i += PAGE4K)
        mmu_kmap(vaddr + i, params.initrd_phys_start + i, PAGE_PR | PAGE_RW);

    genparams.initrd_start = vaddr;
    genparams.initrd_end = vaddr + (params.initrd_phys_end - params.initrd_phys_start);

    //if (params.graphics)
    //{
        unsigned int pages = (params.fbwidth * params.fbheight * (params.fbbpp / 8)) / PAGE4K + 1;
        uintptr_t virt = mmu_kalloc(pages);

        for (unsigned int i = 0; i < pages; i++)
            mmu_kmap(virt + i * PAGE4K, params.fb_phys_addr + i * PAGE4K, PAGE_PR | PAGE_RW);

        fb_set_addr((void*)virt);
    /*}
    else
    {
        uintptr_t virt = mmu_kalloc(8);
        for (unsigned int i = 0; i < 8; i++)
            mmu_kmap(virt + i * PAGE4K, 0xb8000 + i * PAGE4K, PAGE_PR | PAGE_RW);
        vga_set_addr(virt);
    }*/

    printk("done\n");

    printk("loading gdt and idt...");
    gdt_init_cpu(&g_cpus[0]);
    idt_init();
    idt_init_cpu(&g_cpus[0]);
    printk("done\n");

    printk("initializing fpu...");
    fpu_init();
    printk("done\n");
    
    printk("initializing kernel heap...");
    heap_init();
    printk("done\n");

    printk("initializing ACPI...");
    acpi_init(params.rsdp);
    printk("done\n");

    printk("initializing APIC...");
    lapic_setup();
    ioapic_init();
    printk("done\n");

    printk("initializing timer...");
    timer_init();
    printk("done\n");

    sti();
    
    printk("starting cpus...");
    smp_init();
    printk("done\n");

    generic_init(genparams);
}