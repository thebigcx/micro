#pragma once

#include <arch/mmu.h>
#include <micro/mman.h>

// struct pagemap defined in arch/mmu.h
struct vm_map
{
    struct pagemap* pagemap;
    struct list areas;
};

struct vm_object;

struct vmo_ops
{
    // Copy the object to a new map
    void (*copy)(struct vm_object*, struct vm_map*);
    void (*free)(struct vm_object*);
    
    // Handle a page-fault
    int (*fault)(struct vm_object*, uintptr_t);
};

// Virtual Memory Object
struct vm_object
{
    struct vmo_ops ops;
    int flags;
    uintptr_t* pages;

    struct vm_area* area;
    struct vm_map*  map;
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
    struct vm_object* obj;
};

struct vm_area* vm_do_mmap(struct vm_map* map, uintptr_t addr, size_t len, int prot, int flags, int fd, off_t off);

struct vm_map* alloc_vmmap();
struct vm_map* fork_vmmap(struct vm_map* src);
void free_vmmap(struct vm_map* map);

void vm_set_curr(struct vm_map* map);

struct vm_area* vm_map_area(struct vm_map* map, uintptr_t base, size_t size, int fixed);
void vm_unmap_area(struct vm_map* map, struct vm_area* area);

int vm_handle_fault(struct vm_map* map, uintptr_t addr);

struct vm_object* alloc_anon_vmo(struct vm_area* area, int prot);
