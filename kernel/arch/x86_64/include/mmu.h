#pragma once

#include <mmu_defs.h>

#define invlpg(a) asm volatile ("invlpg (%0)" :: "m"(a) : "memory")

void mmu_init();
void mmu_kalloc(struct pml* p, unsigned int flags);
void mmu_kfree(struct pml* p);
void mmu_kmap(uintptr_t virt, uintptr_t phys, int cnt);

uintptr_t mmu_map_mmio(uintptr_t mmio);

uintptr_t mmu_alloc_phys_at(uintptr_t p, unsigned int cnt);
uintptr_t mmu_alloc_phys();
void mmu_free_phys(uintptr_t p, unsigned int cnt);
