#include <boot/protocols.h>
#include <types.h>
#include <debug/syslog.h>
#include <descs.h>
#include <init.h>

static uint8_t stack[4096];

static union gdtent s_gdt[7];
static struct tss s_tss;

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

    struct st2_tag* tag = st2->tags;
    while (tag != NULL)
    {
        switch (tag->id)
        {
            case ST2_TAG_FB_ID:
            {
                struct st2_fbinfo* info = (struct st2_fbinfo*)tag;
                *((uint32_t*)info->addr) = 0xffffffff;
                break;
            }
        }

        tag = tag->next;
    }

    gdt_init(s_gdt, &s_tss);
    kmain();

    for (;;); 
}
