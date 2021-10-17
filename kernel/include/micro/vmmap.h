#pragma once

#include <arch/mmu.h>

// struct pagemap defined in arch/mmu.h
/*struct vm_map
{
    struct pagemap pagemap;
    struct list area;
};

struct vmo_ops
{
    // Copy the object to a new map
    void (*copy)(struct vm_object*, struct vm_map*);
    void (*free)(struct vm_object*);
    
    // Handle a page-fault
    void (*fault)(struct vm_object*, uintptr_t);
};

// Virtual Memory Object
struct vm_object
{
    struct vmo_ops ops;
    int flags;
    uintptr_t* pages;
};

// Anonymous - not backed by file
struct anon_vmo
{
    struct vm_object obj;
};

// Inode - backed by an inode
struct inode_vmo
{
    struct vm_object obj;
    struct inode* ino;
};

// Virtual memory region
struct vm_area
{
    uintptr_t base, end;
    struct vm_object* vmo;
};

struct vm_map* alloc_vmmap();
struct vm_map* fork_vmmap(struct vm_map* src);
void free_vmmap(struct vm_map* map);

void vm_set_curr(struct vm_map* map);

struct vm_area* vm_map_area(struct vm_map* map, uintptr_t base, size_t size);
void vm_unmap_area(struct vm_map* map, struct vm_area* area);

int vm_handle_fault(struct vm_map* map, uintptr_t addr);
*/
