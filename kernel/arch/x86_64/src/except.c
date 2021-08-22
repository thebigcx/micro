#include <except.h>
#include <cpu.h>
#include <debug/syslog.h>

static void panic()
{
    asm volatile ("hlt");
}

static void dump(struct regs* r)
{
    uint64_t cpuid;
    asm volatile ("cpuid" : "=b"(cpuid) : "a"(1));
    dbglnf("cpuid=%x", cpuid >> 24);
    dbglnf("rax=%x", r->rax);
    dbglnf("rbx=%x", r->rbx);
    dbglnf("rcx=%x", r->rcx);
    dbglnf("rdx=%x", r->rdx);
    dbglnf("r8=%x", r->r8);
    dbglnf("r9=%x", r->r9);
    dbglnf("r10=%x", r->r10);
    dbglnf("r11=%x", r->r11);
    dbglnf("r12=%x", r->r12);
    dbglnf("r13=%x", r->r13);
    dbglnf("r14=%x", r->r14);
    dbglnf("r15=%x", r->r15);
    dbglnf("rbp=%x", r->rbp);
    dbglnf("rsp=%x", r->rsp);
    dbglnf("rdi=%x", r->rdi);
    dbglnf("rsi=%x", r->rsi);
    dbglnf("rip=%x", r->rip);
    dbglnf("cs=%x", r->cs);
    dbglnf("ss=%x", r->ss);
    dbglnf("rflags=%x", r->rflags);
    dbglnf("cr0=%x", read_cr0());
    dbglnf("cr2=%x", read_cr2());
    dbglnf("cr3=%x", read_cr3());
    dbglnf("cr4=%x", read_cr4());
}

struct frame
{
    struct frame* rbp;
    uintptr_t rip;
};

static void backtrace(uintptr_t rip, uintptr_t rbp, uint32_t maxframes)
{
    struct frame* frame = (struct frame*)rbp;

    dbgln("Stack trace:");
    dbglnf("    0x%x", rip);

    for (uint32_t i = 0; i < maxframes; i++)
    {
        if (frame == NULL) return;
        dbglnf("    0x%x", frame->rip);
        frame = frame->rbp;
    }
}

static void gp(struct regs* regs, uint32_t e)
{
    dbgln("General protection fault");
    dump(regs);
    backtrace(regs->rip, regs->rbp, 32);
    panic();
}

static void pf(struct regs* regs, uint32_t e)
{
    dbgln("Page fault");
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
            dbglnf("Fatal exception: %d", n);
            dump(regs);
            backtrace(regs->rip, regs->rbp, 32);
            panic();
            break;
    };
}
