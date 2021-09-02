#include <arch/mmu.h>
#include <arch/mmu_defs.h>
#include <arch/cpu_func.h>
#include <micro/stdlib.h>
#include <micro/heap.h>

#define KBASE      (0xffffffff80000000)
#define HEAPBASE   (0xffffffffc0000000)
#define MMIO_BASE  (KBASE - 0x100000000)

#define ENTCNT 512

// Indices of page structures
#define PML4_IDX(vaddr) ((((uint64_t)vaddr) >> 39) & 0x1ff)
#define PDPT_IDX(vaddr) ((((uint64_t)vaddr) >> 30) & 0x1ff)
#define PD_IDX(vaddr)   ((((uint64_t)vaddr) >> 21) & 0x1ff)
#define PT_IDX(vaddr)   ((((uint64_t)vaddr) >> 12) & 0x1ff)

// Kernel: 2M pages located in pml4[511], pdpt[510]
static pml_t kpml4 _pagealign;
static pml_t kpdpt _pagealign;
static pml_t kpd _pagealign;

// Heap: 2M pages located in pml4[511], pdpt[511]
static pml_t kheap_dir _pagealign;
static pml_t kheap_tbls[512] _pagealign;

void mmu_phys_init();

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

    lcr3((uintptr_t)&kpml4 - KBASE);
}

static uintptr_t heap_base = HEAPBASE;

// TODO: rename to 'ksbrk' to reflect what actually happens
uintptr_t mmu_kalloc(size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        unsigned int pdidx = PD_IDX(heap_base);
        unsigned int ptidx = PT_IDX(heap_base);

        if (!(kheap_dir[pdidx] & PAGE_PR))
        {
            kheap_dir[pdidx] = ((uintptr_t)&kheap_tbls[pdidx] - KBASE) | PAGE_PR | PAGE_RW;
        }

        kheap_tbls[pdidx][ptidx] = PAGE_PR | PAGE_RW;

        heap_base += PAGE4K;
    }

    return heap_base - n * PAGE4K;
}

void mmu_kfree(size_t n)
{
    heap_base -= n * PAGE4K;
}

void mmu_kmap(uintptr_t virt, uintptr_t phys, unsigned int flags)
{
    kheap_tbls[PD_IDX(virt)][PT_IDX(virt)] = (phys & PAGE_FRAME) | flags;
    invlpg(virt);
}

static void mktable(struct vm_map* map, unsigned int pdptidx, unsigned int pdidx)
{
    pml_t* table = (pml_t*)mmu_kalloc(1);
    uintptr_t table_phys = mmu_alloc_phys();
    mmu_kmap((uintptr_t)table, table_phys, PAGE_PR | PAGE_RW);
    memset(table, 0, PAGE4K);

    map->pds[pdptidx][pdidx] = table_phys | PAGE_PR | PAGE_RW | PAGE_USR;
    map->pts[pdptidx][pdidx] = (page_t*)table;
}

void mmu_map(struct vm_map* map, uintptr_t virt, uintptr_t phys, unsigned int flags)
{
    unsigned int pdptidx = PDPT_IDX(virt);
    unsigned int pdidx = PD_IDX(virt);
    unsigned int ptidx = PT_IDX(virt);

    if (!(map->pds[pdptidx][pdidx] & PAGE_PR))
    {
        mktable(map, pdptidx, pdidx);
    }

    map->pts[pdptidx][pdidx][ptidx] = phys | flags | PAGE_USR; // Make sure user flag set
    invlpg(virt);
}

// TODO: size parameter for MMIO larger than a page
uintptr_t mmu_map_mmio(uintptr_t mmio)
{
    uintptr_t v = mmu_kalloc(1);
    mmu_kmap(v, mmio, PAGE_PR | PAGE_RW | PAGE_NOCACHE | PAGE_WTHRU);
    return v + mmio % PAGE4K;
}

#define BUF_SZ 156250
static uint8_t phys_bmp[BUF_SZ]; // 0 = free, 1 = used

void mmu_phys_init()
{
    size_t i = 0;
    while (i < BUF_SZ) phys_bmp[i++] = 0xff;
}

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

struct vm_map* mmu_create_vmmap()
{
    struct vm_map* map = kmalloc(sizeof(struct vm_map));

    map->pml4 = (pml_t*)mmu_kalloc(1);
    map->pdpt = (pml_t*)mmu_kalloc(1);
    map->pds = (page_t**)mmu_kalloc(1);
    map->pts = (page_t***)mmu_kalloc(1);

    map->pml4_phys = mmu_alloc_phys();
    map->pdpt_phys = mmu_alloc_phys();

    mmu_kmap((uintptr_t)map->pml4, map->pml4_phys, PAGE_PR | PAGE_RW);
    mmu_kmap((uintptr_t)map->pdpt, map->pdpt_phys, PAGE_PR | PAGE_RW);
    mmu_kmap((uintptr_t)map->pds, mmu_alloc_phys(), PAGE_PR | PAGE_RW);
    mmu_kmap((uintptr_t)map->pts, mmu_alloc_phys(), PAGE_PR | PAGE_RW);

    memcpy(map->pml4, kpml4, PAGE4K);
    memset(map->pdpt, 0, PAGE4K);

    (*map->pml4)[0] = map->pdpt_phys | PAGE_PR | PAGE_RW | PAGE_USR;

    for (unsigned int i = 0; i < ENTCNT; i++)
    {
        map->pds[i] = (page_t*)mmu_kalloc(1);
        map->phys_pds[i] = mmu_alloc_phys();
        mmu_kmap((uintptr_t)map->pds[i], map->phys_pds[i], PAGE_PR | PAGE_RW);
        memset(map->pds[i], 0, PAGE4K);

        (*map->pdpt)[i] = map->phys_pds[i] | PAGE_PR | PAGE_RW | PAGE_USR;

        map->pts[i] = (page_t**)kmalloc(4096); // FIXME: rely on page faults to allocate only what is needed
		memset(map->pts[i], 0, PAGE4K);
    }

    return map;
}

struct vm_map* mmu_clone_vmmap(const struct vm_map* src)
{
    struct vm_map* map = mmu_create_vmmap();

    // Copy over the pages if marked as user, or reference them if they are kernel
    for (unsigned int i = 0; i < ENTCNT; i++)
    for (unsigned int j = 0; j < ENTCNT; j++)
    {
        page_t* srcpt = src->pts[i][j];

        if (srcpt)
        {
            // TODO: COPY THE DATA OVER!!!
            mktable(map, i, j);

            for (unsigned int k = 0; k < ENTCNT; k++)
            {
                if (srcpt[k] & PAGE_PR)
                {
                    if (srcpt[k] & PAGE_USR)
                    {
                        uintptr_t virt1 = mmu_kalloc(1);
                        uintptr_t phys1 = srcpt[k] & PAGE_FRAME;

                        uintptr_t virt2 = mmu_kalloc(1);
                        uintptr_t phys2 = mmu_alloc_phys();

                        mmu_kmap(virt1, phys1, PAGE_PR | PAGE_RW);
                        mmu_kmap(virt2, phys2, PAGE_PR | PAGE_RW);

                        memcpy(virt2, virt1, PAGE4K);
                        // TODO: free the kernel allocated page

                        map->pts[i][j][k] = (~PAGE_FRAME & srcpt[k]) | phys2;
                    }
                    else
                    {
                        srcpt[k] = srcpt[k];
                    }
                }
            }
        }
        else
        {
            map->pds[i][j] = 0;
            map->pts[i][j] = NULL;
        }
    }

    return map;
}

void mmu_destroy_vmmap(struct vm_map* map)
{
    // TODO: implement
}