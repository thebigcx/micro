#include <arch/boot.h>
#include <micro/types.h>
#include <micro/debug.h>
#include <arch/descs.h>
#include <micro/init.h>
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
#include <micro/fbdev.h>
#include <arch/fpu.h>

static uint8_t stack[4096];

static struct st2_header_term termtag =
{
    .tag =
    {
        .id = ST2_TERM_ID,
        .next = 0
    },
    .flags = 0
};

/*static struct st2_header_fb fbtag =
{
    .tag =
    {
        .id = ST2_FB_ID,
        .next = (uintptr_t)&termtag
    },
    .width  = 0,
    .height = 0,
    .bpp    = 0
};*/

SECTION(".stivale2hdr") USED
static struct st2header header =
{
    .entry = 0,
    .stack = (uintptr_t)stack + sizeof(stack),
    .flags = 0,
    //.tags = (uintptr_t)&fbtag
    .tags = (uintptr_t)&termtag
};

term_write_t term_write;
int use_boot_term;

term_write_t find_term(struct st2_tag* tag)
{
    while (tag != NULL)
    {
        if (tag->id == ST2_TAG_TERM_ID)
            return (term_write_t)((struct st2_tag_term*)tag)->term_write;

        tag = (struct st2_tag*)tag->next;
    }

    return NULL;
}

void kmain_st2(struct st2struct* st2)
{
    struct bootparams params;

    //printk("initializing fpu...");
    fpu_init();
    //printk("done\n");
 
    mmu_phys_init();

    term_write = find_term((struct st2_tag*)st2->tags);
    use_boot_term = 0;
    term_write("found terminal tag\n", 19);

    struct st2_tag* tag = (struct st2_tag*)st2->tags;
    while (tag != NULL)
    {
        switch (tag->id)
        {
            case ST2_TAG_FB_ID:
            {
                struct st2_fbinfo* fb = (struct st2_fbinfo*)tag;

                params.fb_phys_addr = fb->addr;
                params.fbwidth = fb->width;
                params.fbheight = fb->height;
                params.fbbpp = fb->depth;

                // TODO: make this better
                fb_init(fb->width, fb->height, fb->depth);
                fb_set_addr((void*)fb->addr);
                fb_set_phys(fb->addr);
                fb_clear(0x0);
                use_boot_term = 0;

                break;
            }
            
            case ST2_TAG_RSDP_ID:
            {
                struct st2_tag_rsdp* rsdp_tag = (struct st2_tag_rsdp*)tag;
                params.rsdp = rsdp_tag->rsdp;
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
                            printk("mapping: %x-%x\n", ent->base, ent->base + ent->length);
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
                
                for (uint64_t i = 0; i < modules->module_cnt; i++)
                {
                    struct st2_module mod = modules->modules[i];

                    // TODO: check to make sure it is the initrd
                    params.initrd_phys_start = mod.begin;
                    params.initrd_phys_end = mod.end;
                }
                
                break;
            }
            case ST2_TAG_CMDLINE_ID:
            {
                struct st2_tag_cmdline* cmdline = (struct st2_tag_cmdline*)tag;
                strcpy(params.cmdline, (const char*)cmdline->cmdline);
                break;
            }
        }

        tag = (struct st2_tag*)tag->next;
    }
 
    printk("finished parsing bootloader tags\n");

    main(params);
}
