#include <arch/except.h>
#include <arch/cpu.h>
#include <micro/debug.h>
#include <arch/reg.h>
#include <arch/cpu_func.h>
#include <micro/task.h>
#include <arch/panic.h>
#include <micro/sched.h>
#include <arch/mmu.h>

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

void unrecoverable(const char* msg, struct regs* regs)
{
    printk("%s\n", msg);
    dump(regs);
    backtrace(regs->rip, regs->rbp, 32);
    panic("Exception in Ring 3");
}

#define UNRECOVER(n, msg)\
    void exception##n(struct regs* regs)\
    {\
        unrecoverable(msg, regs);\
    }

static void divbyzero(struct regs* regs)
{
    if (regs->cs & 3)
    {
        task_send(task_curr(), SIGFPE);
        return;
    }

    printk("Divide by zero\n");
    dump(regs);
    backtrace(regs->rip, regs->rbp, 32);
    panic("Exception in Ring 3");
}

static void debug(struct regs* regs)
{

}

static void nonmask(struct regs* regs)
{

}

static void breakpoint(struct regs* regs)
{

}

static void overflow(struct regs* regs)
{

}

UNRECOVER(5, "Bound range exceeded")

static void invalid_opcode(struct regs* regs)
{
    if (regs->cs & 3)
    {
        dump(regs);
        backtrace(regs->rip, regs->rbp, 32);
        task_send(task_curr(), SIGILL);
        return;
    }

    printk("Invalid opcode\n");
    dump(regs);
    backtrace(regs->rip, regs->rbp, 32);
    panic("Exception in Ring 3");
}

UNRECOVER(7, "Device not available")
UNRECOVER(8, "Double fault")
UNRECOVER(9, "Segment overrun")
UNRECOVER(10, "Invalid TSS")
UNRECOVER(11, "Segment not present")
UNRECOVER(12, "Stack-segment fault")

static void gp(struct regs* regs, uint32_t e)
{
    if (regs->cs & 3)
    {
        dump(regs);
        backtrace(regs->rip, regs->rbp, 32);
        task_send(task_curr(), SIGSEGV);
        return;
    }

    printk("General protection fault\n");
    dump(regs);
    backtrace(regs->rip, regs->rbp, 32);
    panic("Exception in Ring 3");
}

static void pf(struct regs* regs, uint32_t e)
{
    if (regs->cs & 3) // TODO: can also fault allocate when doing ABI stuff like stack setup
    {
        if (vm_map_handle_fault(task_curr()->vm_map, rcr2()) == -1)
        {
            dump(regs);
            backtrace(regs->rip, regs->rbp, 32);
            task_send(task_curr(), SIGSEGV); // Could not handle
        }

        return;
    }

    printk("Page fault\n");
    dump(regs);
    backtrace(regs->rip, regs->rbp, 32);
    panic("Exception in Ring 3");
}

static void x87_error(struct regs* regs)
{
    if (regs->cs & 3)
    {
        task_send(task_curr(), SIGFPE);
        return;
    }

    printk("x87 error\n");
    dump(regs);
    backtrace(regs->rip, regs->rbp, 32);
    panic("Exception in Ring 3");
}

UNRECOVER(17, "Alignment check")
UNRECOVER(18, "Machine check")

static void simd_error(struct regs* regs)
{
    if (regs->cs & 3)
    {
        task_send(task_curr(), SIGFPE);
        return;
    }

    printk("SIMD error\n");
    dump(regs);
    backtrace(regs->rip, regs->rbp, 32);
    panic("Exception in Ring 3");
}

static void virt_except(struct regs* regs)
{

}

static void (*handlers[])() =
{
    divbyzero,
    debug,
    nonmask,
    breakpoint,
    overflow,
    exception5,
    invalid_opcode,
    exception7,
    exception8,
    exception9,
    exception10,
    exception11,
    exception12,
    gp,
    pf,
    NULL,
    x87_error,
    exception17,
    exception18,
    simd_error,
    virt_except,
    
};

void except(uintptr_t n, struct regs* regs, uint32_t e)
{
    static int nested = 0;
    nested++;
    if (nested > 1)
    {
        //if (regs->cs & 3)
        {
            // Something went VERY wrong
            task_send(task_curr(), SIGILL);
            nested = 0;
            sched_yield();
        }
        //else
            //panic("Nested exceptions");
    }

    handlers[n](regs);

    // If it returned, then safe task termination was all that was required
    nested--;
    sched_yield();
}
