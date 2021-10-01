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

        struct vm_area* area = vm_map_anon(task_curr()->vm_map, addr, length, flags & MAP_FIXED);
        vm_map_anon_alloc(task_curr()->vm_map, area, addr, length); // TODO: TEMP (SHOULD NOT ALLOCATE)
    }
    else
    {
        // TEMP
        if (flags & MAP_SHARED) return (unsigned long)-EINVAL;

        FDVALID(fdno);

        struct fd* fd = task_curr()->fds[fdno];

        if (fd->filp->ops.mmap)
        {
            struct vm_area* area = vm_map_anon(task_curr()->vm_map, addr, length, flags & MAP_FIXED);
            
            vfs_mmap(fd->filp, area);
        }
        else
        {
            if (fd->filp->type != S_IFREG) return (unsigned long)-EACCES;

            unsigned int mmu_flags = PAGE_PR;
            mmu_flags |= prot & PROT_WRITE ? PAGE_RW : 0;

            for (uintptr_t i = (uintptr_t)addr; i < (uintptr_t)addr + length; i += PAGE4K)
            {
                mmu_map(task_curr()->vm_map, i, mmu_alloc_phys(), mmu_flags);
            }

            vfs_read(fd->filp, addr, 0, fd->filp->size);
        }
    }

    return (unsigned long)addr;
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