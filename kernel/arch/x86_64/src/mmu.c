#include <mmu.h>
#include <mmu_defs.h>

#define KBASE      (0xffffffff80000000)
#define PAGE_FRAME (0xFFFFffffFFFFf000)
#define MMIO_BASE  (KBASE - 0x100000000)

#define ENTCNT 512

// TODO: struct pml should just be a typedef of uint64_t[512]

// Kernel: 2M pages located in pml4[511], pdpt[511]
static struct pml kpml4;
static struct pml kpdpt;
static struct pml kpd;

static struct pml kheap_dir;
static struct pml kheap_tbls[512];

void mmu_init()
{
    memset(&kpml4, 0, sizeof(struct pml));
    memset(&kpdpt, 0, sizeof(struct pml));

    kpml4.entries[511] = ((uintptr_t)&kpdpt - KBASE) | PAGE_PR | PAGE_RW;
    kpml4.entries[0] = kpml4.entries[511];
    kpdpt.entries[510] = ((uintptr_t)&kpd - KBASE) | PAGE_PR | PAGE_RW;

    for (int i = 0; i < ENTCNT; i++)
        kpd.entries[i] = (PAGE2M * i) | PAGE_PR | PAGE_RW | PD_2M;

    kpdpt.entries[0] = kpdpt.entries[510];

    uintptr_t cr3 = (uintptr_t)&kpml4 - KBASE;
    asm ("mov %0, %%cr3" :: "r"(cr3));
}

void mmu_kalloc(struct pml* p, unsigned int flags)
{

}

void mmu_kfree(struct pml* p)
{

}

void mmu_kmap(uintptr_t virt, uintptr_t phys, int cnt)
{

}

uintptr_t mmu_map_mmio(uintptr_t mmio)
{
    // TODO: impl
}

#define BUF_SZ 15625
static uint8_t phys_bmp[BUF_SZ]; // 0 = free, 1 = used

uintptr_t mmu_alloc_phys_at(uintptr_t p, unsigned int cnt)
{
    uintptr_t strt = p / PAGE4K;

    for (uintptr_t i = strt; i < strt + cnt; i++)
        phys_bmp[i / 8] |= (1 << (i % 8));

    return p;
}

uintptr_t mmu_alloc_phys()
{
    for (unsigned long i = 0; i < BUF_SZ; i++)
    {
        if (phys_bmp[i] != 0xff) // At least one free
        {
            for (int j = 0; j < 8; j++)
            {
                if (!(phys_bmp[i] & (1 << j))) // Check if bit is 0
                {
                    uintptr_t addr = (i * 8 + j) * PAGE4K;
                    return mmu_alloc_phys_at(addr, 1);
                }
            }
        }
    }

    return 0;
}

void mmu_free_phys(uintptr_t p, unsigned int cnt)
{
    uintptr_t strt = p / PAGE4K;
    uintptr_t end = p / PAGE4K + cnt;

    for (uintptr_t it = strt; it < end; it++)
    {
        unsigned long i = it % 8;
        unsigned long j = it / 8;

        phys_bmp[j] &= ~(1 << i); // Unset the bit
    }
}
