#include <mmu.h>
#include <mmu_defs.h>
#include <stdlib.h>

#define KBASE      (0xffffffff80000000)
#define HEAPBASE   (0xffffffffc0000000)
#define PAGE_FRAME (0xFFFFffffFFFFf000)
#define MMIO_BASE  (KBASE - 0x100000000)

#define ENTCNT 512

// Indices of page structures
#define PML4_IDX(vaddr) ((((uint64_t)vaddr) >> 39) & 0x1ff)
#define PDPT_IDX(vaddr) ((((uint64_t)vaddr) >> 30) & 0x1ff)
#define PD_IDX(vaddr)   ((((uint64_t)vaddr) >> 21) & 0x1ff)
#define PT_IDX(vaddr)   ((((uint64_t)vaddr) >> 12) & 0x1ff)

#define _pagealign __attribute__((aligned(PAGE4K)))

// Kernel: 2M pages located in pml4[511], pdpt[510]
static pml_t kpml4 _pagealign;
static pml_t kpdpt _pagealign;
static pml_t kpd _pagealign;

// Heap: 2M pages located in pml4[511], pdpt[511]
static pml_t kheap_dir _pagealign;
static pml_t kheap_tbls[512] _pagealign;

void mmu_init()
{
    memset(&kpml4, 0, sizeof(pml_t));
    memset(&kpdpt, 0, sizeof(pml_t));

    kpml4[511] = ((uintptr_t)&kpdpt - KBASE) | PAGE_PR | PAGE_RW;
    kpml4[0] = kpml4[511];
    kpdpt[510] = ((uintptr_t)&kpd - KBASE) | PAGE_PR | PAGE_RW;

    for (int i = 0; i < ENTCNT; i++)
        kpd[i] = (PAGE2M * i) | PAGE_PR | PAGE_RW | PD_2M;

    kpdpt[0] = kpdpt[510];

    kpdpt[511] = ((uintptr_t)&kheap_dir - KBASE) | PAGE_PR | PAGE_RW;
    for (int i = 0; i < ENTCNT; i++)
        memset(&(kheap_tbls[i]), 0, sizeof(page_t) * ENTCNT);

    uintptr_t cr3 = (uintptr_t)&kpml4 - KBASE;
    asm ("mov %0, %%cr3" :: "r"(cr3));
}

uintptr_t mmu_kalloc()
{
    static uintptr_t heap_base = HEAPBASE;

    unsigned int pdidx = PD_IDX(heap_base);
    unsigned int ptidx = PT_IDX(heap_base);

    if (!(kheap_dir[pdidx] & PAGE_PR))
    {
        kheap_dir[pdidx] = ((uintptr_t)&kheap_tbls[pdidx] - KBASE) | PAGE_PR | PAGE_RW;
    }

    kheap_tbls[pdidx][ptidx] = PAGE_PR | PAGE_RW;

    heap_base += PAGE4K;
    return heap_base - PAGE4K;
}

void mmu_kfree(uintptr_t p)
{
    (void)p;
}

void mmu_kmap(uintptr_t virt, uintptr_t phys, unsigned int flags)
{
    kheap_tbls[PD_IDX(virt)][PT_IDX(virt)] = (phys & PAGE_FRAME) | flags;
    invlpg(virt);
}

uintptr_t mmu_map_mmio(uintptr_t mmio)
{
    uintptr_t v = mmu_kalloc();
    mmu_kmap(v, mmio, PAGE_PR | PAGE_RW | PAGE_NOCACHE | PAGE_WTHRU);
    return v + mmio % PAGE4K;
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
