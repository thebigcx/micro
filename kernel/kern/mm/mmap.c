#include <micro/sys.h>
#include <micro/mman.h>
#include <micro/vfs.h>
#include <arch/mmu.h>

unsigned long ksys_do_mmap(void* addr, size_t length, int prot, int flags, int fdno, off_t offset)
{
    if (!length) return (unsigned long)-EINVAL;
    if (!(flags & MAP_SHARED) && !(flags & MAP_PRIVATE)) return (unsigned long)-EINVAL;

    if (flags & MAP_ANONYMOUS)
    {
        // TEMP
        if (flags & MAP_SHARED) return (unsigned long)-EINVAL;
        
        if ((uintptr_t)addr < task_curr()->brk)
            addr = (void*)task_curr()->brk;

        struct vm_area* area = vm_map_anon(task_curr()->vm_map, (uintptr_t)addr, length, flags & MAP_FIXED);
        vm_map_anon_alloc(task_curr()->vm_map, area, area->base, length); // TODO: TEMP (SHOULD NOT ALLOCATE)
        return area->base;
    }
    else
    {
        // TEMP
        if (flags & MAP_SHARED) return (unsigned long)-EINVAL;

        FDVALID(fdno);

        struct file* fd = task_curr()->fds[fdno];

        if (fd->ops.mmap)
        {
            struct vm_area* area = vm_map_anon(task_curr()->vm_map, (uintptr_t)addr, length, flags & MAP_FIXED);
            vfs_mmap(fd, area);
            return area->base;
        }
        else
        {
            // TEMP
            return 0;
            
            if (!S_ISREG(fd->inode->mode)) return (unsigned long)-EACCES;

            //unsigned int mmu_flags = PAGE_PR;
            //mmu_flags |= prot & PROT_WRITE ? PAGE_RW : 0;
            unsigned int mmu_flags = PAGE_PR | PAGE_RW; // TODO: this is dangerous

            for (uintptr_t i = (uintptr_t)addr; i < (uintptr_t)addr + length; i += PAGE4K)
            {
                mmu_map(task_curr()->vm_map, i, mmu_alloc_phys(), mmu_flags);
            }

            vfs_read(fd, addr, fd->inode->size);
        }
    }
    
    // Unreachable
    return (unsigned long)-EINVAL;
}

// TODO: add support for shared memory
// TODO: use the 'offset' parameter
SYSCALL_DEFINE(mmap, void* addr, size_t length, int prot, int flags, int fdno, off_t offset)
{
    return ksys_do_mmap(addr, length, prot, flags, fdno, offset);
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
    ksys_do_mmap((void*)old, (size_t)(addr - old), 0, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    return task_curr()->brk = (uintptr_t)addr;
}

SYSCALL_DEFINE(mprotect, void* addr, size_t len, int prot)
{
    return 0;
}
