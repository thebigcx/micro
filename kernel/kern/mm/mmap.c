#include <micro/sys.h>
#include <micro/mman.h>
#include <micro/vfs.h>
#include <arch/mmu.h>
#include <micro/vmmap.h>

struct vm_area* vm_map_anon(struct vm_map* map, uintptr_t base, size_t len, int prot, int flags)
{
    struct vm_area* area = vm_map_area(map, base, len, flags & MAP_FIXED);
    if (!area) return NULL;

    area->obj = alloc_anon_vmo(area, prot);
    area->obj->map = map;
    return area;
}

struct vm_area* vm_do_mmap(struct vm_map* map, uintptr_t addr, size_t length, int prot, int flags, int fdno, off_t offset)
{
    if (flags & MAP_ANONYMOUS)
    {
        return vm_map_anon(map, addr, length, prot, flags);
    }
    else
    {
        struct file* file = task_curr()->fds[fdno];

        if (file->ops.mmap)
        {
            return 0;
            //return vm_map_file(task_curr()->vm_map, (uintptr_t)addr, length, flags & MAP_FIXED, file)->base;
        }
        else
        {
            // TEMP
            return 0;
        }
    }
}

// TODO: add support for shared memory
// TODO: use the 'offset' parameter
SYSCALL_DEFINE(mmap, void* addr, size_t length, int prot, int flags, int fdno, off_t offset)
{
    if (!length) return (unsigned long)-EINVAL;
    if (!(flags & MAP_SHARED) && !(flags & MAP_PRIVATE)) return (unsigned long)-EINVAL;
    if (!(flags & MAP_ANON)) FDVALID(fdno);

    if ((flags & MAP_ANON) && (uintptr_t)addr < task_curr()->brk)
        addr = (void*)task_curr()->brk;

    return vm_do_mmap(task_curr()->vm_map, (uintptr_t)addr, length, prot, flags, fdno, offset)->base;
}

SYSCALL_DEFINE(munmap, void* addr, size_t length)
{
    if (!length) return -EINVAL;
    if ((uintptr_t)addr % PAGE4K != 0) return -EINVAL;

    // TODO: unmap the physical blocks if anonymous
    return 0;
}

// TODO: reduce program break
SYSCALL_DEFINE(brk, void* addr)
{
    if (!addr)
        return task_curr()->brk;
   
    uintptr_t old = task_curr()->brk;
    vm_do_mmap(task_curr()->vm_map, (void*)old, (size_t)(addr - old), 0, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    return task_curr()->brk = (uintptr_t)addr;
}

SYSCALL_DEFINE(mprotect, void* addr, size_t len, int prot)
{
    return 0;
}
