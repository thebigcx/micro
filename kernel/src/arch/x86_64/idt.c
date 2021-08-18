#include <descs.h>

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

void idt_init()
{

}

void idt_init_cpu(struct cpu_info* cpu)
{
    struct descptr idtr;
    idtr.lim = sizeof(s_idt) - 1;
    idtr.base = (uintptr_t)s_idt;

    lidt(idtr);
}
