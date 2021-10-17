#include <micro/vmmap.h>
#include <micro/heap.h>

/*struct vm_map* alloc_vmmap()
{
    struct vm_map* map = kcalloc(sizeof(struct vm_map));

    map->areas = list_create();
    map->pagemap = mmu_create_pagemap();

    return map;
}

struct vm_map* fork_vmmap(struct vm_map* src)
{
    
}

void free_vmmap(struct vm_map* map)
{
    mmu_destroy_pagemap(map->pagemap);
    kfree(map);
}

void vm_set_curr(struct vm_map* map)
{
    mmu_set_currmap(map->pagemap);
}

struct vm_area* vm_map_area(struct vm_map* map, uintptr_t base, size_t size)
{

}

void vm_unmap_area(struct vm_map* map, struct vm_area* area)
{

}

int vm_handle_fault(struct vm_map* map, uintptr_t addr)
{

}*/
