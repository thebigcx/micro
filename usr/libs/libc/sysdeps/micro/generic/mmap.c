#include <sys/mman.h>
#include <libc/syscall.h>

void* mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    return (void*)SYSCALL_ERR(mmap, addr, length, prot, flags, fd, offset);
}