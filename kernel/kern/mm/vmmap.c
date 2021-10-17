#include <micro/vmmap.h>
#include <micro/heap.h>

struct vm_map* alloc_vmmap()
{
    struct vm_map* map = kcalloc(sizeof(struct vm_map));

    map->areas   = list_create();
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

struct vm_area* area_create(uintptr_t base, uintptr_t end, struct vm_object* obj)
{
    struct vm_area* area = kmalloc(sizeof(struct vm_area));

    area->base = base;
    area->end  = end;
    area->obj  = obj;
    
    return area;
}

static struct vm_area* area_allocat(struct vm_map* map, uintptr_t base, size_t size)
{
    uintptr_t end = base + size;

    uint32_t i = 0;
    LIST_FOREACH(&map->areas)
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
            return list_insert_before(&map->areas, i, area_create(base, end, NULL));
    }

    // Push it back
    if (!map->areas.size
     ||((struct vm_area*)list_back(&map->areas))->end <= base)
    {
        return list_enqueue(&map->areas, area_create(base, end, NULL));
    }

    return NULL;

}

static struct vm_area* area_alloc(struct vm_map* map, size_t size)
{
    uintptr_t base = PAGE4K;
    uintptr_t end = base + size;

    uint32_t i = 0;
    LIST_FOREACH(&map->areas)
    {
        struct vm_area* area = node->data;

        if (area->base >= end)
            return list_insert_before(&map->areas, i, area_create(base, end, NULL));

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
        return list_enqueue(&map->areas, area_create(base, end, NULL));

    return NULL;
}

struct vm_area* vm_map_area(struct vm_map* map, uintptr_t base, size_t size, int fixed)
{
    if (base)
    {
        struct vm_area* area = area_allocat(map, base, size);
        if (area) return area;
        else
        {
            if (fixed) return NULL;
            else return area_alloc(map, size); // Fallback
        }
    }
    else return area_alloc(map, size);
}

void vm_unmap_area(struct vm_map* map, struct vm_area* area)
{

}

int vm_handle_fault(struct vm_map* map, uintptr_t addr)
{
    LIST_FOREACH(&map->areas)
    {
        struct vm_area* area = node->data;
        if (addr >= area->base && addr <= area->end)
            return area->obj->ops.fault(area->obj, addr);
    }

    return -1;
}

static int anonvmo_fault(struct vm_object* obj, uintptr_t addr)
{
    if (addr % PAGE4K)
        addr -= addr % PAGE4K;

    size_t i = (addr - obj->area->base) / PAGE4K;
    obj->pages[i] = mmu_alloc_phys();
    mmu_map(obj->map, addr, obj->pages[i], PAGE_PR | PAGE_RW);
  
    printk("fault: %x\n", addr);
    return 0;
}

static struct vmo_ops s_anonvmo_ops =
{
    .fault = anonvmo_fault
};

struct vm_object* alloc_anon_vmo(struct vm_area* area, int prot)
{
    struct anon_vmo* vmo = kcalloc(sizeof(struct anon_vmo));

    vmo->obj.flags = MAP_ANON;
    vmo->obj.ops   = s_anonvmo_ops;
    vmo->obj.pages = kmalloc(sizeof(uintptr_t) * (area->end - area->base) / PAGE4K);
    vmo->obj.area  = area;

    return vmo;
}
