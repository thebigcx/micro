#include <boot/protocols.h>
#include <types.h>
#include <debug/syslog.h>
#include <descs.h>
#include <init.h>
#include <cpu.h>
#include <mmu.h>
#include <acpi.h>
#include <lapic.h>
#include <smp.h>
#include <heap.h>

static uint8_t stack[4096];

static struct st2_header_fb fbtag =
{
    .tag =
    {
        .id = ST2_FB_ID,
        .next = 0
    },
    .width  = 0,
    .height = 0,
    .bpp    = 0
};

__attribute__((section(".stivale2hdr"), used))
static struct st2header header =
{
    .entry = 0,
    .stack = (uintptr_t)stack + sizeof(stack),
    .flags = 0,
    .tags = (uintptr_t)&fbtag
};

void kmain_st2(struct st2struct* st2)
{
    dbgln("entry");

    // TEST
    gdt_init_cpu(&g_cpus[0]);
    idt_init();
    idt_init_cpu(&g_cpus[0]);

    dbgln("loaded gdt");

    mmu_init();
    
    dbgln("loaded cr3");
 
    uintptr_t rsdp = 0;
    uintptr_t initrd_start = 0, initrd_end = 0;

    struct st2_tag* tag = (struct st2_tag*)st2->tags;
    while (tag != NULL)
    {
        switch (tag->id)
        {
            case ST2_TAG_FB_ID:
                break;
            case ST2_TAG_RSDP_ID:
            {
                struct st2_tag_rsdp* rsdp_tag = (struct st2_tag_rsdp*)tag;
                rsdp = rsdp_tag->rsdp;
                break;
            }
            case ST2_TAG_MMAP_ID:
            {
                struct st2_tag_mmap* mmaptag = (struct st2_tag_mmap*)tag;

                for (uint32_t i = 0; i < mmaptag->entries; i++)
                {
                    struct st2_mmap_ent* ent = &mmaptag->mmap[i];
                    
                    switch (ent->type)
                    {
                        case ST2_MMAP_USABLE:
                        case ST2_MMAP_BOOTLD_RECL:
                            mmu_free_phys(ent->base, ent->length / PAGE4K);
                            break;
                        
                        default:
                            break;
                    }
                }

                break;
            }
            case ST2_TAG_MODS_ID:
            {
                struct st2_tag_mods* modules = (struct st2_tag_mods*)tag;
                
                struct st2_module mod = modules->modules[0];
                
                for (uint64_t i = 0; i < modules->module_cnt; i++)
                {
                    struct st2_module mod = modules->modules[i];

                    uintptr_t vaddr = mmu_kalloc((mod.end - mod.begin + PAGE4K) / PAGE4K);
                    
                    for (size_t i = 0; i < (mod.end - mod.begin + PAGE4K) / PAGE4K; i++)
                        mmu_kmap(vaddr + i * PAGE4K, mod.begin + i * PAGE4K, PAGE_PR | PAGE_RW);

                    //if (strcmp(mod.string, "initrd") == 0)
                    {
                        initrd_start = vaddr;
                        initrd_end = vaddr + (mod.end - mod.begin);
                    }
                }
                
                break;
            }
        }

        tag = (struct st2_tag*)tag->next;
    }
 
    mmu_alloc_phys_at(0, 0x100);
    
    heap_init();

    acpi_init(rsdp);
    acpi_parse_madt();

    lapic_setup();

    timer_init();

    sti();
    
    smp_init(acpi_get_lapics());

    dbgln("starting scheduler");
    sched_init();

    //kmain();

    for (;;); 
}
