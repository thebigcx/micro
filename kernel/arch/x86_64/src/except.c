#include <except.h>
#include <cpu.h>
#include <debug/syslog.h>
#include <reg.h>

static void panic()
{
    asm volatile ("hlt");
}

static void dump(struct regs* r)
{
    uint64_t cpuid;
    asm volatile ("cpuid" : "=b"(cpuid) : "a"(1));
    printk("cpuid=%x", cpuid >> 24);
    printk("rax=%x", r->rax);
    printk("rbx=%x", r->rbx);
    printk("rcx=%x", r->rcx);
    printk("rdx=%x", r->rdx);
    printk("r8=%x", r->r8);
    printk("r9=%x", r->r9);
    printk("r10=%x", r->r10);
    printk("r11=%x", r->r11);
    printk("r12=%x", r->r12);
    printk("r13=%x", r->r13);
    printk("r14=%x", r->r14);
    printk("r15=%x", r->r15);
    printk("rbp=%x", r->rbp);
    printk("rsp=%x", r->rsp);
    printk("rdi=%x", r->rdi);
    printk("rsi=%x", r->rsi);
    printk("rip=%x", r->rip);
    printk("cs=%x", r->cs);
    printk("ss=%x", r->ss);
    printk("rflags=%x", r->rflags);
    printk("cr0=%x", rcr0());
    printk("cr2=%x", rcr2());
    printk("cr3=%x", rcr3());
    printk("cr4=%x", rcr4());
}

struct frame
{
    struct frame* rbp;
    uintptr_t rip;
};

static void backtrace(uintptr_t rip, uintptr_t rbp, uint32_t maxframes)
{
    struct frame* frame = (struct frame*)rbp;

    printk("Stack trace:");
    printk("    0x%x", rip);

    for (uint32_t i = 0; i < maxframes; i++)
    {
        if (frame == NULL) return;
        printk("    0x%x", frame->rip);
        frame = frame->rbp;
    }
}

static void gp(struct regs* regs, uint32_t e)
{
    printk("General protection fault");
    dump(regs);
    backtrace(regs->rip, regs->rbp, 32);
    panic();
}

static void pf(struct regs* regs, uint32_t e)
{
    printk("Page fault");
    dump(regs);
    backtrace(regs->rip, regs->rbp, 32);
    panic();
}

void except(uintptr_t n, struct regs* regs, uint32_t e)
{
    switch (n)
    {
        case 13:
            gp(regs, e);
            break;
        case 14:
            pf(regs, e);
            break;
        default:
            printk("Fatal exception: %d", n);
            dump(regs);
            backtrace(regs->rip, regs->rbp, 32);
            panic();
            break;
    };
}
