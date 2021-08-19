#include <descs.h>
#include <idt_stubs.h>

#define REGISTER_ISR(i) mkintr(i, isr##i, 0)
#define REGISTER_IRQ(i) mkintr(i + 32, irq##i, 0)

#define FOLD8(x, i) x(i+0);x(i+1);x(i+2);x(i+3);x(i+4);x(i+5);x(i+6);x(i+7);
#define FOLD16(x, i) FOLD8(x,i);FOLD8(x,i+8);
#define FOLD32(x, i) FOLD8(x,i);FOLD8(x,i+8);FOLD8(x,i+16);FOLD8(x,i+24);

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

void isr_handler()
{

}

void irq_handler()
{

}

void idt_init()
{
    FOLD32(REGISTER_ISR, 0);
    FOLD16(REGISTER_IRQ, 0);

    mkintr(0x80, irq0x80, 1); // syscall()

    REGISTER_IRQ(0xfd);
    REGISTER_IRQ(0xfe);
}

void idt_init_cpu(struct cpu_info* cpu)
{
    struct descptr idtr;
    idtr.lim = sizeof(s_idt) - 1;
    idtr.base = (uintptr_t)s_idt;

    lidt(idtr);
}
