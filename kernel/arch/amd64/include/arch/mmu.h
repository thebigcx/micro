#pragma once

#include <arch/mmu_defs.h>

// User Virtual-Memory map
struct vm_map
{
    pml_t* pml4;
    uintptr_t pml4_phys;

    pml_t* pdpt;
    uintptr_t pdpt_phys;

    page_t** pds; // page_t*[512]
    uintptr_t phys_pds[512];

    page_t*** pts;
};

void mmu_init();
uintptr_t mmu_kalloc(size_t n);
void mmu_kfree(uintptr_t ptr, size_t n);
void mmu_kmap(uintptr_t virt, uintptr_t phys, unsigned int flags);

void mmu_map(struct vm_map* map, uintptr_t virt, uintptr_t phys, unsigned int flags);

uintptr_t mmu_map_mmio(uintptr_t mmio);

void mmu_phys_init();
uintptr_t mmu_alloc_phys_at(uintptr_t p, unsigned int cnt);
uintptr_t mmu_alloc_phys();
void mmu_free_phys(uintptr_t p, unsigned int cnt);

uintptr_t mmu_virt2phys(struct vm_map* map, uintptr_t virt);

struct vm_map* mmu_create_vmmap();
struct vm_map* mmu_clone_vmmap(const struct vm_map* src);
void mmu_destroy_vmmap(struct vm_map* map);