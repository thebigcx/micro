#include <arch/mmu.h>
#include <arch/mmu_defs.h>
#include <arch/cpu_func.h>
#include <micro/stdlib.h>
#include <micro/heap.h>
#include <micro/debug.h>
#include <micro/vfs.h>

#define ENTCNT 512

// Indices of page structures
#define PML4_IDX(vaddr) ((((uint64_t)vaddr) >> 39) & 0x1ff)
#define PDPT_IDX(vaddr) ((((uint64_t)vaddr) >> 30) & 0x1ff)
#define PD_IDX(vaddr)   ((((uint64_t)vaddr) >> 21) & 0x1ff)
#define PT_IDX(vaddr)   ((((uint64_t)vaddr) >> 12) & 0x1ff)

// Kernel: 2M pages located in pml4[511], pdpt[510]
static pml_t kpml4 PAGEALIGN;
static pml_t kpdpt PAGEALIGN;
static pml_t kpd   PAGEALIGN;

// Heap: 2M pages located in pml4[511], pdpt[511]
static pml_t kheap_dir       PAGEALIGN;
static pml_t kheap_tbls[512] PAGEALIGN;

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

    mmu_set_kpml4();
}

uintptr_t mmu_kalloc(size_t n)
{
    unsigned int counter = 0;
    uint64_t ptidx = 0;
    uint64_t pdidx = 0;

    // Attempt to use pre-allocated structures
    for (uint32_t i = 0; i < ENTCNT; i++)
    {
        if (!(kheap_dir[i] & PAGE_PR) || kheap_dir[i] & PD_2M)
        {
            pdidx = i + 1;
            ptidx = 0;
            counter = 0;
            continue;
        }

        for (uint32_t j = 0; j < ENTCNT; j++)
        {
            if (kheap_tbls[i][j] & PAGE_PR)
            {
                pdidx = i;
                ptidx = j + 1;
                counter = 0;
                continue;
            }

            counter++;

            if (counter >= n)
            {
                uintptr_t addr = HEAPBASE + (pdidx * PAGE2M) + (ptidx * PAGE4K);

                // Allocate the pages
                while (counter--)
                {
                    if (ptidx >= 512)
                    {
                        pdidx++;
                        ptidx = 0;
                    }

                    kheap_tbls[pdidx][ptidx] = PAGE_PR;
                    ptidx++;
                }

                return addr;
            }
        }
    }

    counter = 0;
    pdidx = 0;
    ptidx = 0;

    // Need to allocate more paging structures

    for (uint32_t i = 0; i < ENTCNT; i++)
    {
        if (kheap_dir[i] & PAGE_PR)
        {
            pdidx = i + 1;
            ptidx = 0;
            counter = 0;
            continue;
        }

        counter += 512;
        if (counter >= n)
        {
            uintptr_t addr = HEAPBASE + (pdidx * PAGE2M) + (ptidx * PAGE4K);

            kheap_dir[pdidx] = ((uintptr_t)&kheap_tbls[pdidx] - KBASE) | PAGE_PR | PAGE_RW;

            // Allocate the pages
            while (n--)
            {
                if (ptidx >= 512)
                {
                    pdidx++;
                    ptidx = 0;
                    kheap_dir[pdidx] = ((uintptr_t)&kheap_tbls[pdidx] - KBASE) | PAGE_PR | PAGE_RW;
                }

                kheap_tbls[pdidx][ptidx] = PAGE_PR;
                ptidx++;
            }

            return addr;
        }
    }

    printk("mmu_kalloc(): failed to allocate virtual memory of size=%x\n", n * PAGE4K);
    return 0;
}

void mmu_kfree(uintptr_t ptr, size_t n)
{
    while (n--)
    {
        kheap_tbls[PD_IDX(ptr)][PT_IDX(ptr)] = 0; // Non-present
        invlpg(ptr); // Flush TLB
        ptr += PAGE4K;
    }
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

uintptr_t mmu_map_mmio(uintptr_t mmio, size_t cnt)
{
    uintptr_t v = mmu_kalloc(cnt);

    for (uintptr_t i = 0; i < cnt; i++)
        mmu_kmap(v + i * PAGE4K, mmio, PAGE_PR | PAGE_RW | PAGE_NOCACHE | PAGE_WTHRU);
    
    return v + mmio % PAGE4K;
}

uintptr_t mmu_map_module(size_t size)
{
    // Assure page-alignment
    if (size % PAGE4K) size += PAGE4K - (size % PAGE4K);

    size_t cnt = size / PAGE4K;

    uintptr_t v = mmu_kalloc(cnt);

    for (uintptr_t i = 0; i < cnt; i++)
        mmu_kmap(v + i * PAGE4K, mmu_alloc_phys(), PAGE_PR | PAGE_RW);
    
    return v;
}

void mmu_unmap_module(uintptr_t base, size_t size)
{
    // Assure page-alignment
    if (size % PAGE4K) size += PAGE4K - (size % PAGE4K);

    size_t cnt = size / PAGE4K;

    for (uintptr_t i = 0; i < cnt; i++)
        mmu_free_phys(mmu_kvirt2phys(base + i * PAGE4K), 1);

    mmu_kfree(base, cnt);
}

uintptr_t mmu_kvirt2phys(uintptr_t virt)
{
    if (PML4_IDX(virt) != 511 || PDPT_IDX(virt) != 511) return 0;

    unsigned int pdi = PD_IDX(virt);
    unsigned int pti = PT_IDX(virt);

    return kheap_tbls[pdi][pti] & PAGE_FRAME;
}

// TODO: detect memory and resize buffer
#define BUF_SZ 200000
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

    printk("pmm: out of physical memory pages.\n");
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

uintptr_t mmu_virt2phys(struct vm_map* map, uintptr_t virt)
{
    // Indices
    uint32_t pml4i = PML4_IDX(virt);
    uint32_t pdpti = PDPT_IDX(virt);
    uint32_t pdi = PD_IDX(virt);
    uint32_t pti = PT_IDX(virt);

    // User address space
    if (pml4i == 0)
    {
        if (!(map->pds[pdpti][pdi] & PAGE_PR) || !(map->pts[pdpti][pdi])) return 0;
        return map->pts[pdpti][pdi][pti] & PAGE_FRAME;
    }
    else
        return 0; // Either kernel space or completely invalid
}

struct vm_map* mmu_create_vmmap()
{
    struct vm_map* map = kmalloc(sizeof(struct vm_map));

    map->vm_areas = list_create();

    map->pml4 = (pml_t*)mmu_kalloc(1);
    map->pdpt = (pml_t*)mmu_kalloc(1);
    map->pds = (page_t**)kmalloc(PAGE4K);
    map->pts = (page_t***)kmalloc(PAGE4K);

    map->pml4_phys = mmu_alloc_phys();
    map->pdpt_phys = mmu_alloc_phys();

    mmu_kmap((uintptr_t)map->pml4, map->pml4_phys, PAGE_PR | PAGE_RW);
    mmu_kmap((uintptr_t)map->pdpt, map->pdpt_phys, PAGE_PR | PAGE_RW);

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

        map->pts[i] = (page_t**)kmalloc(4096); // FIXME: create the page tables as needed
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

                        memcpy((void*)virt2, (const void*)virt1, PAGE4K);
                        
                        mmu_kfree(virt1, 1);
                        mmu_kfree(virt2, 1);

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
    for (unsigned int i = 0; i < ENTCNT; i++)
    {
        for (unsigned int j = 0; j < ENTCNT; j++)
        {
            if (map->pts[i][j])
            {
                for (unsigned int k = 0; k < ENTCNT; k++)
                {
                    if (   map->pts[i][j][k] & PAGE_PR
                        && map->pts[i][j][k] & PAGE_USR) // Don't want to delete kernel pages
                    {
                        mmu_free_phys(map->pts[i][j][k] & PAGE_FRAME, 1);
                    }
                }

                mmu_kfree((uintptr_t)map->pts[i][j], 1);
                mmu_free_phys(map->pds[i][j] & PAGE_FRAME, 1);
            }
        }

        kfree(map->pts[i]);

        mmu_kfree((uintptr_t)map->pds[i], 1);
        mmu_free_phys(map->phys_pds[i], 1);
    }

    mmu_kfree((uintptr_t)map->pml4, 1);
    mmu_kfree((uintptr_t)map->pdpt, 1);
    mmu_free_phys(map->pml4_phys, 1);
    mmu_free_phys(map->pdpt_phys, 1);

    kfree(map->pds);
    kfree(map->pts);

    kfree(map);
}

struct vm_area* vm_area_create(uintptr_t base, uintptr_t end, struct vm_object* obj)
{
    struct vm_area* area = kmalloc(sizeof(struct vm_area));

    area->base = base;
    area->end  = end;
    area->obj  = obj;
    
    return area;
}

// TODO: move this stuff to a seperate file
struct vm_area* vm_map_alloc(struct vm_map* map, size_t size)
{
    uintptr_t base = PAGE4K;
    uintptr_t end = base + size;

    uint32_t i = 0;
    LIST_FOREACH(&map->vm_areas)
    {
        struct vm_area* area = node->data;

        if (area->base >= end)
        {
            return list_insert(&map->vm_areas, i, vm_area_create(base, end, NULL));
        }

        // Intersects/Encapsulates
        if ((base >= area->base && base <  area->end)
         || (end  >  area->base && end  <= area->end)
         || (base <  area->base && end  >  area->end))
        {
            base = area->end;
            end = base + size;
        }

        i++;
    }

    if (end < KBASE)
    {
        return list_push_back(&map->vm_areas, vm_area_create(base, end, NULL));
    }

    return NULL;
}

struct vm_area* vm_map_allocat(struct vm_map* map, uintptr_t base, size_t size)
{
    uintptr_t end = base + size;

    uint32_t i = 0;
    LIST_FOREACH(&map->vm_areas)
    {
        struct vm_area* area = node->data;

        if (area->end <= base)
        {
            i++;
            continue;
        }

        if (area->base >= end)
        {
            return list_insert(&map->vm_areas, i, vm_area_create(base, end, NULL));
        }
    }

    // Push it back
    if (!map->vm_areas.size
     ||((struct vm_area*)list_back(&map->vm_areas))->end <= base)
    {
        return list_push_back(&map->vm_areas, vm_area_create(base, end, NULL));
    }

    return NULL;
}

// Create a private anonymous mapping
struct vm_area* vm_map_anon(struct vm_map* map, uintptr_t base, size_t size, int fixed)
{
    struct vm_area* area;

    if (base)
    {
        area = vm_map_allocat(map, base, size);
        if (!area)
        {
            if (fixed) return NULL;
            else area = vm_map_alloc(map, size); // Fallback
        }
    }
    else
        area = vm_map_alloc(map, size);

    struct anon_vmo* obj = kmalloc(sizeof(struct anon_vmo));

    obj->flags = ANON_PRIVATE;
    obj->pages = kmalloc(sizeof(uintptr_t) * size / PAGE4K);

    area->obj = (struct vm_object*)obj;
    area->obj->type = VMO_ANON;

    return area;
}

struct vm_area* vm_map_file(struct vm_map* map, uintptr_t base, int fixed, struct file* file)
{
    return NULL; // TODO: implement
}

// Allocate part of an anonymous mapping
void vm_map_anon_alloc(struct vm_map* map, struct vm_area* area, uintptr_t base, size_t size)
{
    base -= area->base;

    struct anon_vmo* anon = area->obj;

    for (uintptr_t i = base / PAGE4K; i < (base + size) / PAGE4K; i++)
    {
        anon->pages[i] = mmu_alloc_phys();
        mmu_map(map, i * PAGE4K + area->base, anon->pages[i], PAGE_PR | PAGE_RW);
    }
}

// Handle a page fault - if it cannot, returns -1
int vm_map_handle_fault(struct vm_map* map, uintptr_t addr)
{
    // Page-align address
    if (addr % PAGE4K)
        addr -= addr % PAGE4K;

    LIST_FOREACH(&map->vm_areas)
    {
        struct vm_area* area = node->data;

        // addr inside the vm_area
        if (addr >= area->base && addr + PAGE4K <= area->end)
        {
            if (area->obj->type == VMO_ANON)
            {
                struct anon_vmo* anon = area->obj;
                
                unsigned int i = (addr - area->base) / PAGE4K;
                anon->pages[i] = mmu_alloc_phys();

                mmu_map(map, addr, anon->pages[i], PAGE_PR | PAGE_RW);
                return 0;
            }
        }
    }

    return -1;
}

void mmu_set_kpml4()
{
    lcr3((uintptr_t)&kpml4 - KBASE);
}