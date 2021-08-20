#include <boot/protocols.h>
#include <types.h>
#include <debug/syslog.h>
#include <descs.h>
#include <init.h>
#include <cpu.h>

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

    struct st2_tag* tag = (struct st2_tag*)st2->tags;
    while (tag != NULL)
    {
        switch (tag->id)
        {
            case ST2_TAG_FB_ID:
                break;
        }

        tag = (struct st2_tag*)tag->next;
    }

    mmu_alloc_phys_at(0, 0x100);

    // TEST
    struct cpu_info bsp;
    gdt_init_cpu(&bsp);
    idt_init();
    idt_init_cpu(&bsp);

    dbgln("loaded gdt");

    mmu_init();

    dbgln("loaded cr3");
   
    kmain();

    for (;;); 
}
