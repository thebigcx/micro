#include <descs.h>
#include <stdlib.h>
#include <cpu.h>
#include <cpu_func.h>

#define TYPE_DATA_W 0x2 // Writable Data
#define TYPE_CODE_X 0x8 // Executable Code
#define TYPE_TSS    0x9 // TSS segment

static union gdtent s_gdt[] =
{
    { .low = 0, .high = 0 },
    { .low = 0xffff, .high = 0xaf9800 },
    { .low = 0xffff, .high = 0xaf9300 },
    { .low = 0xffff, .high = 0xaff800 },
    { .low = 0xffff, .high = 0xaff200 },
    { .low = 0, .high = 0xe900 }, // TSS
    { .low = 0, .high = 0 }  // TSS Extended
};

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
    memcpy(gdt, s_gdt, sizeof(union gdtent) * 7);
    
    setbase(&gdt[5], (uintptr_t)tss & 0xffffffff);
    setlim(&gdt[5], sizeof(struct tss) - 1);

    gdt[6].low = (uintptr_t)tss >> 32;
    gdt[6].high = 0;

    memset(tss, 0, sizeof(struct tss));
    tss->iomap = sizeof(struct tss);
}

void gdt_init_cpu(struct cpu_info* cpu)
{
    mkgdt(cpu->gdt, &cpu->tss);

    struct descptr gdtr;
    gdtr.lim = 7 * sizeof(union gdtent) - 1;
    gdtr.base = (uintptr_t)cpu->gdt;

    lgdt_full(&gdtr, GDT_CODE0, GDT_DATA0);
    ltr(GDT_TSS | 3);
}
