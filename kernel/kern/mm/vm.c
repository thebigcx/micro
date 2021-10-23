#include <micro/vm.h>
#include <arch/mmu.h>
#include <micro/heap.h>

// TODO: remove
#include <micro/vfs.h>

struct vm_map* vm_alloc_map()
{
    struct vm_map* map = kcalloc(sizeof(struct vm_map));
    mmu_init_vmmap(map);

    map->vm_areas = list_create();

    return map;
}

struct vm_map* vm_clone_map(const struct vm_map* src)
{
    struct vm_map* map = kcalloc(sizeof(struct vm_map));
    mmu_clone_vmmap(src, map);

    map->vm_areas = list_create(); // TODO: clone them properly: copy-on-write, shared-memory, etc etc
    
    return map;
}

void vm_free_map(struct vm_map* map)
{
    mmu_free_vmmap(map);
    
    // TODO: free areas properly i.e. copy-on-write, shared-memory, etc

    list_free(&map->vm_areas);
    kfree(map);
}

static struct vm_area* vm_area_create(uintptr_t base, uintptr_t end, struct vm_map* map)
{
    struct vm_area* area = kcalloc(sizeof(struct vm_area));

    area->base = base;
    area->end  = end;
    area->map  = map;
    
    return area;
}

struct vm_area* vm_map_alloc(struct vm_map* map, size_t size)
{
    uintptr_t base = PAGE4K;
    uintptr_t end = base + size;

    uint32_t i = 0;
    LIST_FOREACH(&map->vm_areas)
    {
        struct vm_area* area = node->data;

        if (area->base >= end)
        {
            return list_insert_before(&map->vm_areas, i, vm_area_create(base, end, map));
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
        return list_enqueue(&map->vm_areas, vm_area_create(base, end, map));
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
            return list_insert_before(&map->vm_areas, i, vm_area_create(base, end, map));
        }
    }

    // Push it back
    if (!map->vm_areas.size
     ||((struct vm_area*)list_back(&map->vm_areas))->end <= base)
    {
        return list_enqueue(&map->vm_areas, vm_area_create(base, end, map));
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

static int anonvmo_fault(struct vm_area* area, uintptr_t addr)
{
    unsigned int i = (addr - area->base) / PAGE4K;
    area->obj->pages[i] = mmu_alloc_phys();

    printk("fault: map=%x addr=%x base=%x i=%d\n", area->map, addr, area->base, i);
    mmu_map(area->map, addr, area->obj->pages[i], PAGE_PR | PAGE_RW);
    return 0;
}

// Create a private anonymous mapping
struct vm_area* vm_map_anon(struct vm_map* map, uintptr_t base, size_t size, int fixed)
{
    struct vm_area* area = alloc_vmarea(map, base, size, fixed);
    
    area->obj = kmalloc(sizeof(struct vm_object));

    area->obj->pages = kmalloc(sizeof(uintptr_t) * size / PAGE4K);
    area->obj->flags = VM_ANON | VM_PRIV;

    area->obj->fault = anonvmo_fault;

    return area;
}

struct vm_area* vm_map_file(struct vm_map* map, uintptr_t base, size_t size, int fixed, struct file* file)
{
    if (size % PAGE4K)
        size += (PAGE4K - size % PAGE4K);

    struct vm_area* area = alloc_vmarea(map, base, size, fixed);
    
    if (file->ops.mmap)
        file->ops.mmap(file, area);
    else
    {
        // TODO
        return NULL;
    }

    area->obj = kmalloc(sizeof(struct vm_object));
   
    area->obj->flags = VM_PRIV;
    area->obj->pages = kmalloc(sizeof(uintptr_t) * size / PAGE4K);
    area->obj->inode = file->inode;
    
    return area;
}

// Allocate part of an anonymous mapping
void vm_map_anon_alloc(struct vm_map* map, struct vm_area* area, uintptr_t base, size_t size)
{
    base -= area->base;

    for (uintptr_t i = base / PAGE4K; i < (base + size) / PAGE4K; i++)
    {
        area->obj->pages[i] = mmu_alloc_phys();
        mmu_map(map, i * PAGE4K + area->base, area->obj->pages[i], PAGE_PR | PAGE_RW);
    }
}

// Handle a page fault - if it cannot, returns -1
int vm_map_handle_fault(struct vm_map* map, uintptr_t addr)
{
    // Page-align address
    if (addr % PAGE4K)
        addr -= addr % PAGE4K;

    // Find the vm_area that 'addr' is inside
    LIST_FOREACH(&map->vm_areas)
    {
        struct vm_area* area = node->data;

        // addr inside the vm_area
        if (addr >= area->base && addr + PAGE4K <= area->end)
            return area->obj->fault(area, addr);
    }

    // Invalid memory access
    return -1;
}

