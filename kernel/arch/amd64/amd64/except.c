#include <except.h>
#include <cpu.h>
#include <debug/syslog.h>
#include <reg.h>
#include <cpu_func.h>

static void dump(struct regs* r)
{
    uint64_t cpuid;
    asm volatile ("cpuid" : "=b"(cpuid) : "a"(1));
    printk("cpuid=%x\n", cpuid >> 24);
    printk("rax=%x\n", r->rax);
    printk("rbx=%x\n", r->rbx);
    printk("rcx=%x\n", r->rcx);
    printk("rdx=%x\n", r->rdx);
    printk("r8=%x\n", r->r8);
    printk("r9=%x\n", r->r9);
    printk("r10=%x\n", r->r10);
    printk("r11=%x\n", r->r11);
    printk("r12=%x\n", r->r12);
    printk("r13=%x\n", r->r13);
    printk("r14=%x\n", r->r14);
    printk("r15=%x\n", r->r15);
    printk("rbp=%x\n", r->rbp);
    printk("rsp=%x\n", r->rsp);
    printk("rdi=%x\n", r->rdi);
    printk("rsi=%x\n", r->rsi);
    printk("rip=%x\n", r->rip);
    printk("cs=%x\n", r->cs);
    printk("ss=%x\n", r->ss);
    printk("rflags=%x\n", r->rflags);
    printk("cr0=%x\n", rcr0());
    printk("cr2=%x\n", rcr2());
    printk("cr3=%x\n", rcr3());
    printk("cr4=%x\n", rcr4());
}

struct frame
{
    struct frame* rbp;
    uintptr_t rip;
};

static void backtrace(uintptr_t rip, uintptr_t rbp, uint32_t maxframes)
{
    struct frame* frame = (struct frame*)rbp;

    printk("Stack trace:\n");
    printk("    0x%x\n", rip);

    for (uint32_t i = 0; i < maxframes; i++)
    {
        if (frame == NULL) return;
        printk("    0x%x\n", frame->rip);
        frame = frame->rbp;
    }
}

static void gp(struct regs* regs, uint32_t e)
{
    printk("General protection fault\n");
    dump(regs);
    backtrace(regs->rip, regs->rbp, 32);
    panic();
}

static void pf(struct regs* regs, uint32_t e)
{
    printk("Page fault\n");
    dump(regs);
    backtrace(regs->rip, regs->rbp, 32);
    panic();
}

void except(uintptr_t n, struct regs* regs, uint32_t e)
{
    static int nested = 0;
    nested++;
    if (nested > 1)
    {
        panic("Nested exceptions");
    }

    switch (n)
    {
        case 13:
            gp(regs, e);
            break;
        case 14:
            pf(regs, e);
            break;
        default:
            printk("Fatal exception: %d\n", n);
            dump(regs);
            backtrace(regs->rip, regs->rbp, 32);
            panic();
            break;
    };
}
