#pragma once

#include <arch/mmu.h>

#define VMO_ANON  0
#define VMO_INODE 1

// TODO: refactor all of this

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

struct inode;
struct file;

#define INODE_PRIVATE 0

struct inode_vmo
{
    struct vm_object obj;
    struct inode*    inode;
    uintptr_t*       pages;
    int              flags;
};

struct vm_area
{
    uintptr_t base;
    uintptr_t end;
    struct vm_object* obj;
};

struct vm_map* vm_alloc_map();
struct vm_map* vm_clone_map(const struct vm_map* src);
void vm_free_map(struct vm_map* map);

struct vm_area* vm_map_alloc(struct vm_map* map, size_t size);
struct vm_area* vm_map_allocat(struct vm_map* map, uintptr_t base, size_t size);

struct vm_area* vm_map_anon(struct vm_map* map, uintptr_t base, size_t size, int fixed);
struct vm_area* vm_map_file(struct vm_map* map, uintptr_t base, size_t size, int fixed, struct file* file);

void vm_map_anon_alloc(struct vm_map* map, struct vm_area* area, uintptr_t base, size_t size);

int vm_map_handle_fault(struct vm_map* map, uintptr_t addr);
