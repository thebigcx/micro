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
#include <cpu_func.h>
#include <vfs.h>
#include <stdlib.h>
#include <task.h>
#include <sched.h>

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

ssize_t tty_read(struct file* file, void* buf, off_t off, size_t size)
{
    memset(buf, '6', size);
    return 0;
}

ssize_t tty_write(struct file* file, const void* buf, off_t off, size_t size)
{
    printk("%s", (char*)buf);
    return size;
}

void kmain_st2(struct st2struct* st2)
{
    printk("entry");

    // TEST
    gdt_init_cpu(&g_cpus[0]);
    idt_init();
    idt_init_cpu(&g_cpus[0]);

    printk("loaded gdt");

    mmu_init();
    
    printk("loaded cr3");
 
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

                    if (!strcmp(mod.string, "initrd"))
                    {
                        initrd_init(vaddr, vaddr + (mod.end - mod.begin));
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

    sys_init();

    vfs_init();

    struct file* file = kmalloc(sizeof(struct file));
    file->ops.read = tty_read;
    file->ops.write = tty_write;
    vfs_mount(file, "/dev/tty");

    //char* relat;
    //struct file* tty = vfs_getmnt("/dev/tty", &relat);
    //ASSERT(file == tty);

    struct fd* fd = vfs_open(vfs_resolve("/dev/tty"));
    printk("%x %x\n", fd->filp, file);

    void* buffer = initrd_read("init");
    struct task* init = task_creat(buffer, NULL, NULL);
    sched_start(init);

//    for(;;);

    printk("starting scheduler");
    sched_init();

    for (;;); 
}
