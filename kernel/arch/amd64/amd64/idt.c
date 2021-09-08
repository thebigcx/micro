#include <arch/descs.h>
#include <arch/intr_stubs.h>
#include <arch/cpu.h>
#include <micro/debug.h>
#include <arch/lapic.h>
#include <arch/timer.h>
#include <arch/except.h>
#include <micro/stdlib.h>
#include <arch/cpu_func.h>

#define REGISTER_ISR(i) mkintr(i, isr##i, 0)
#define REGISTER_IRQ(i) mkintr(i + 32, irq##i, 0)

#define FOLD8(X, i) X(i+0);X(i+1);X(i+2);X(i+3);X(i+4);X(i+5);X(i+6);X(i+7);
#define FOLD16(X, i) FOLD8(X,i);FOLD8(X,i+8);
#define FOLD32(X, i) FOLD8(X,i);FOLD8(X,i+8);FOLD8(X,i+16);FOLD8(X,i+24);

#define TYPE_INTR 0xe
#define TYPE_TRAP 0xf

static struct idtent s_idt[256];
static void (*s_handlers[256])(struct regs*);

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
    if (s_handlers[n]) s_handlers[n](r);
}

void idt_set_handler(unsigned int n, void (*handler)(struct regs*))
{
    ASSERT(n < 256);
    s_handlers[n] = handler;
}

void idt_init()
{
    REGISTER_ISR(0);
    REGISTER_ISR(1);
    REGISTER_ISR(2);
    REGISTER_ISR(3);
    REGISTER_ISR(4);
    REGISTER_ISR(5);
    REGISTER_ISR(6);
    REGISTER_ISR(7);
    REGISTER_ISR(8);
    REGISTER_ISR(9);
    REGISTER_ISR(10);
    REGISTER_ISR(11);
    REGISTER_ISR(12);
    REGISTER_ISR(13);
    REGISTER_ISR(14);
    REGISTER_ISR(15);
    REGISTER_ISR(16);
    REGISTER_ISR(17);
    REGISTER_ISR(18);
    REGISTER_ISR(19);
    REGISTER_ISR(20);
    REGISTER_ISR(21);
    REGISTER_ISR(22);
    REGISTER_ISR(23);
    REGISTER_ISR(24);
    REGISTER_ISR(25);
    REGISTER_ISR(26);
    REGISTER_ISR(27);
    REGISTER_ISR(28);
    REGISTER_ISR(29);
    REGISTER_ISR(30);
    REGISTER_ISR(31);
    
    REGISTER_IRQ(0);
    REGISTER_IRQ(1);
    REGISTER_IRQ(2);
    REGISTER_IRQ(3);
    REGISTER_IRQ(4);
    REGISTER_IRQ(5);
    REGISTER_IRQ(6);
    REGISTER_IRQ(7);
    REGISTER_IRQ(8);
    REGISTER_IRQ(9);
    REGISTER_IRQ(10);
    REGISTER_IRQ(11);
    REGISTER_IRQ(12);
    REGISTER_IRQ(13);
    REGISTER_IRQ(14);
    REGISTER_IRQ(15);

    mkintr(0x80, irq0x80, 1); // syscall()
    mkintr(0xfd, irq0xfd, 0); // schedule()
    mkintr(0xfe, irq0xfe, 0); // halt()
}

void idt_init_cpu(struct cpu_info* cpu)
{
    volatile struct descptr idtr;
    idtr.lim = sizeof(s_idt) - 1;
    idtr.base = (uintptr_t)s_idt;

    lidt(&idtr);
}
