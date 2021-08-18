#include <descs.h>
#include <stdlib.h>
#include <cpu.h>

#define TYPE_DATA_W 0x2 // Writable Data
#define TYPE_CODE_X 0x8 // Executable Code
#define TYPE_TSS    0x9 // TSS segment

static union gdtent mkentry(unsigned int type, int user)
{
    return (union gdtent)
    {
        .limlo     = 0xffff,
        .baselo    = 0,
        .basemid   = 0,
        .type      = (uint32_t)type,
        .dtype     = 1,
        .dpl       = user ? 3 : 0,
        .present   = 1,
        .limhi     = 0xf,
        .avail     = 0,
        .size64    = 1,
        .size32    = 0,
        .gran      = 1,
        .basehi    = 0
    };
}

static void setbase(union gdtent* ent, uint32_t base)
{
    ent->baselo = base & 0xffff;
    ent->basemid = (base >> 16) & 0xff;
    ent->basehi = (base >> 24) & 0xff;
}

static void setlim(union gdtent* ent, uint32_t lim)
{
    ent->limlo = lim & 0xffff;
    ent->limhi = (lim >> 16) & 0xf;
}

static void mkgdt(union gdtent* gdt, struct tss* tss)
{
    gdt[0] = (union gdtent) { .low = 0, .high = 0 };
    gdt[1] = mkentry(TYPE_CODE_X, 0);
    gdt[2] = mkentry(TYPE_DATA_W, 0);
    gdt[3] = mkentry(TYPE_CODE_X, 1);
    gdt[4] = mkentry(TYPE_DATA_W, 1);

    setbase(&gdt[5], (uintptr_t)tss & 0xffffffff);
    setlim(&gdt[5], sizeof(struct tss) - 1);

    gdt[5].type = TYPE_TSS;
    gdt[5].present = 1;
    gdt[5].dpl = 3;

    gdt[6].low = (uintptr_t)tss >> 32;
    gdt[6].high = 0;

    memset(tss, 0, sizeof(struct tss));
    tss->iomap = sizeof(struct tss);
}

extern void lgdt(struct descptr*);
extern void ltr(uint16_t);

void gdt_init_cpu(struct cpu_info* cpu)
{
    mkgdt(cpu->gdt, &cpu->tss);

    struct descptr gdtr;
    gdtr.lim = 7 * sizeof(union gdtent) - 1;
    gdtr.base = (uintptr_t)cpu->gdt;

    lgdt(&gdtr);
    ltr(GDT_TSS | 3);
}
