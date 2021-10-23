#pragma once

#include <arch/mmu.h>

#define VM_ANON 0x1
#define VM_PRIV 0x2

// TODO: refactor all of this

struct inode;
struct vm_area;

struct vm_object
{
    int flags;
    uintptr_t* pages;

    union
    {
        struct inode* inode;
    };

    int (*copy)(struct vm_object*);
    int (*fault)(struct vm_area*, uintptr_t);
    int (*free)(struct vm_object*);
};

struct vm_area
{
    uintptr_t base;
    uintptr_t end;
    struct vm_object* obj;
    struct vm_map* map;
};

struct vm_map* vm_alloc_map();
struct vm_map* vm_clone_map(const struct vm_map* src);
void vm_free_map(struct vm_map* map);

struct vm_area* vm_map_alloc(struct vm_map* map, size_t size);
struct vm_area* vm_map_allocat(struct vm_map* map, uintptr_t base, size_t size);

struct file;

struct vm_area* vm_map_anon(struct vm_map* map, uintptr_t base, size_t size, int fixed);
struct vm_area* vm_map_file(struct vm_map* map, uintptr_t base, size_t size, int fixed, struct file* file);

void vm_map_anon_alloc(struct vm_map* map, struct vm_area* area, uintptr_t base, size_t size);

int vm_map_handle_fault(struct vm_map* map, uintptr_t addr);
