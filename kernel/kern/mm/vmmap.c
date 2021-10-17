#include <micro/vmmap.h>
#include <arch/mmu.h>
#include <micro/vfs.h>

// TODO: this function is fucking SLOW!
struct vm_map* alloc_vmmap()
{
    struct vm_map* map = kmalloc(sizeof(struct vm_map));

    map->pagemap = mmu_create_pagemap();
    map->vm_areas = list_create();

    return map;
}

// TODO: this function is stupid - each mapping should define its own copy() function
struct vm_map* fork_vmmap(const struct vm_map* src)
{
    struct vm_map* map = kcalloc(sizeof(struct vm_map));

    map->pagemap = mmu_clone_pagemap(src->pagemap);

    return map;
}

void free_vmmap(struct vm_map* map)
{
    mmu_destroy_pagemap(map->pagemap);
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
    uintptr_t base = PAGE_SIZE;
    uintptr_t end = base + size;

    uint32_t i = 0;
    LIST_FOREACH(&map->vm_areas)
    {
        struct vm_area* area = node->data;

        if (area->base >= end)
        {
            return list_insert_before(&map->vm_areas, i, vm_area_create(base, end, NULL));
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
        return list_enqueue(&map->vm_areas, vm_area_create(base, end, NULL));
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

        // Intersects/Encapsulates
        if ((base >= area->base && base <  area->end)
         || (end  >  area->base && end  <= area->end)
         || (base <  area->base && end  >  area->end))
        {
            return NULL;
        }
        
        // Lower down
        if (area->end <= base)
        {
            i++;
            continue;
        }
        
        if (area->base >= end)
        {
            return list_insert_before(&map->vm_areas, i, vm_area_create(base, end, NULL));
        }
    }

    // Push it back
    if (!map->vm_areas.size
     ||((struct vm_area*)list_back(&map->vm_areas))->end <= base)
    {
        return list_enqueue(&map->vm_areas, vm_area_create(base, end, NULL));
    }

    return NULL;
}

static struct vm_area* alloc_vmarea(struct vm_map* map, uintptr_t base, size_t size, int fixed)
{
    if (base)
    {
        struct vm_area* area = vm_map_allocat(map, base, size);
        if (area) return area;
        else
        {
            if (fixed) return NULL;
            else return vm_map_alloc(map, size); // Fallback
        }
    }
    else
        return vm_map_alloc(map, size);
}

// Create a private anonymous mapping
struct vm_area* vm_map_anon(struct vm_map* map, uintptr_t base, size_t size, int fixed)
{
    struct vm_area* area = alloc_vmarea(map, base, size, fixed);
    struct anon_vmo* obj = kmalloc(sizeof(struct anon_vmo));

    obj->flags = ANON_PRIVATE;
    obj->pages = kmalloc(sizeof(uintptr_t) * size / PAGE_SIZE);

    area->obj = (struct vm_object*)obj;
    area->obj->type = VMO_ANON;

    return area;
}

struct vm_area* vm_map_file(struct vm_map* map, uintptr_t base, size_t size, int fixed, struct file* file)
{
    if (size % PAGE_SIZE)
        size += (PAGE_SIZE - size % PAGE_SIZE);

    struct vm_area* area = alloc_vmarea(map, base, size, fixed);
    
    if (file->ops.mmap)
        file->ops.mmap(file, area);
    else
    {
        // TODO
        return NULL;
    }

    struct inode_vmo* obj = kmalloc(sizeof(struct inode_vmo));
    
    obj->flags = INODE_PRIVATE;
    obj->inode = file->inode;
    obj->pages = kmalloc(sizeof(uintptr_t) * size / PAGE_SIZE);
    
    area->obj = (struct vm_object*)obj;
    area->obj->type = VMO_INODE;
    
    /*for (uintptr_t i = 0; i < size / PAGE_SIZE; i++)
    {
        obj->pages[i] = mmu_alloc_phys();
        mmu_map(map, area->base + i * PAGE_SIZE, obj->pages[i], PAGE_PR | PAGE_RW);
    }*/
    
    return area;
}

// Allocate part of an anonymous mapping
void vm_map_anon_alloc(struct vm_map* map, struct vm_area* area, uintptr_t base, size_t size)
{
    base -= area->base;

    struct anon_vmo* anon = (struct anon_vmo*)area->obj;

    for (uintptr_t i = base / PAGE_SIZE; i < (base + size) / PAGE_SIZE; i++)
    {
        anon->pages[i] = mmu_alloc_phys();
        mmu_map(map->pagemap, i * PAGE_SIZE + area->base, anon->pages[i], PAGE_PR | PAGE_RW);
    }
}

// Handle a page fault - if it cannot, returns -1
int vm_map_handle_fault(struct vm_map* map, uintptr_t addr)
{
    // Page-align address
    if (addr % PAGE_SIZE)
        addr -= addr % PAGE_SIZE;

    LIST_FOREACH(&map->vm_areas)
    {
        struct vm_area* area = node->data;

        // addr inside the vm_area
        if (addr >= area->base && addr + PAGE_SIZE <= area->end)
        {
            if (area->obj->type == VMO_ANON)
            {
                struct anon_vmo* anon = (struct anon_vmo*)area->obj;
                
                unsigned int i = (addr - area->base) / PAGE_SIZE;
                anon->pages[i] = mmu_alloc_phys();

                printk("fault: map=%x addr=%x base=%x i=%d\n", map, addr, area->base, i);
                mmu_map(map->pagemap, addr, anon->pages[i], PAGE_PR | PAGE_RW);
                return 0;
            }
        }
    }

    return -1;
}

void vm_set_currmap(struct vm_map* map)
{
    mmu_set_pagemap(map->pagemap);
}
