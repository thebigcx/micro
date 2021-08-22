#include <descs.h>
#include <intr_stubs.h>
#include <cpu.h>
#include <debug/syslog.h>
#include <lapic.h>
#include <timer.h>
#include <except.h>

#define REGISTER_ISR(i) mkintr(i, isr##i, 0)
#define REGISTER_IRQ(i) mkintr(i + 32, irq##i, 0)

#define FOLD8(X, i) X(i+0);X(i+1);X(i+2);X(i+3);X(i+4);X(i+5);X(i+6);X(i+7);
#define FOLD16(X, i) FOLD8(X,i);FOLD8(X,i+8);
#define FOLD32(X, i) FOLD8(X,i);FOLD8(X,i+8);FOLD8(X,i+16);FOLD8(X,i+24);

#define TYPE_INTR 0xe
#define TYPE_TRAP 0xf

static struct idtent s_idt[256];

static void mkintr(unsigned int num, void (*handler)(), int user)
{
    s_idt[num] = (struct idtent)
    {
        .offlo = (uintptr_t)handler & 0xffff,
        .select = GDT_CODE0,
        .ist = 0,
        .type_attr =
        {
            .type = TYPE_INTR,
            .store = 0,
            .dpl = user ? 3 : 0,
            .present = 1
        },
        .offmid = ((uintptr_t)handler >> 16) & 0xffff,
        .offhi = ((uintptr_t)handler >> 32) & 0xffffffff,
        .zero = 0
    };
}

void isr_handler(uintptr_t n, struct regs* r, uint32_t e)
{
    lapic_eoi();
    except(n, r, e);
}

void irq_handler(uintptr_t n, struct regs* r)
{
    lapic_eoi();
    if (n == 32) timer_tick(r);
    else if (n == IPI_SCHED) switch_next(r);
}

void idt_init()
{
    FOLD32(REGISTER_ISR, 0);
    FOLD16(REGISTER_IRQ, 0);

    mkintr(0x80, irq0x80, 1); // syscall()
    mkintr(0xfd, irq0xfd, 0); // schedule()
    mkintr(0xfe, irq0xfe, 0); // halt()
}

void idt_init_cpu(struct cpu_info*)
{
    struct descptr idtr;
    idtr.lim = sizeof(s_idt) - 1;
    idtr.base = (uintptr_t)s_idt;

    lidt(&idtr);
}
