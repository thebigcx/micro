#pragma once

#include <arch/mmu_defs.h>
#include <micro/list.h>

// User Virtual-Memory map
struct vm_map
{
    pml_t*    pml4;
    uintptr_t pml4_phys;

    pml_t*    pdpt;
    uintptr_t pdpt_phys;

    page_t**  pds; // page_t*[512]
    uintptr_t phys_pds[512];

    page_t*** pts;

    struct list vm_areas; // List of virtual memory areas
};

#define VMO_ANON 0
#define VMO_FILE 1

// TODO: make union?
struct vm_object
{
    int type;
};

#define ANON_SHARED  0
#define ANON_PRIVATE 1

struct anon_vmo
{
    struct vm_object obj;
    int flags;
    uintptr_t* pages;
};

struct file_vmo
{
    struct vm_object obj;
};

struct vm_area
{
    uintptr_t base;
    uintptr_t end;
    struct vm_object* obj;
};

void mmu_init();
uintptr_t mmu_kalloc(size_t n);
void mmu_kfree(uintptr_t ptr, size_t n);
void mmu_kmap(uintptr_t virt, uintptr_t phys, unsigned int flags);

void mmu_map(struct vm_map* map, uintptr_t virt, uintptr_t phys, unsigned int flags);

uintptr_t mmu_map_mmio(uintptr_t mmio, size_t cnt);
uintptr_t mmu_map_module(size_t size);
void mmu_unmap_module(uintptr_t base, size_t size);

void mmu_phys_init();
uintptr_t mmu_alloc_phys_at(uintptr_t p, unsigned int cnt);
uintptr_t mmu_alloc_phys();
void mmu_free_phys(uintptr_t p, unsigned int cnt);

uintptr_t mmu_virt2phys(struct vm_map* map, uintptr_t virt);
uintptr_t mmu_kvirt2phys(uintptr_t virt);

struct vm_map* mmu_create_vmmap();
struct vm_map* mmu_clone_vmmap(const struct vm_map* src);
void mmu_destroy_vmmap(struct vm_map* map);

struct vm_area* vm_map_alloc(struct vm_map* map, size_t size);
struct vm_area* vm_map_allocat(struct vm_map* map, uintptr_t base, size_t size);

struct vm_area* vm_map_anon(struct vm_map* map, uintptr_t base, size_t size, int fixed);

void vm_map_anon_alloc(struct vm_map* map, struct vm_area* area, uintptr_t base, size_t size);

int vm_map_handle_fault(struct vm_map* map, uintptr_t addr);

void mmu_set_kpml4();