#pragma once

#include <arch/mmu_defs.h>
#include <micro/list.h>

struct pagemap
{
    pml_t*    pml4;
    uintptr_t pml4_phys;

    pml_t*    pdpt;
    uintptr_t pdpt_phys;

    page_t**  pds; // page_t*[512]
    uintptr_t phys_pds[512];

    page_t*** pts;
};

void mmu_init();
uintptr_t mmu_kalloc(size_t n);
void mmu_kfree(uintptr_t ptr, size_t n);
void mmu_kmap(uintptr_t virt, uintptr_t phys, unsigned int flags);

void mmu_map(struct pagemap* map, uintptr_t virt, uintptr_t phys, unsigned int flags);

uintptr_t mmu_map_mmio(uintptr_t mmio, size_t cnt);
uintptr_t mmu_map_module(size_t size);
void mmu_unmap_module(uintptr_t base, size_t size);

void mmu_phys_init();
uintptr_t mmu_alloc_phys_at(uintptr_t p, unsigned int cnt);
uintptr_t mmu_alloc_phys();
void mmu_free_phys(uintptr_t p, unsigned int cnt);

uintptr_t mmu_virt2phys(struct pagemap* map, uintptr_t virt);
uintptr_t mmu_kvirt2phys(uintptr_t virt);

void mmu_set_kpml4();

struct pagemap* mmu_create_pagemap();
struct pagemap* mmu_clone_pagemap(const struct pagemap* src);
void mmu_destroy_pagemap(struct pagemap* map);
