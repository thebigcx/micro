#include <sys/mman.h>
#include <libc/syscall.h>

int munmap(void* addr, size_t length)
{
    return SYSCALL_ERR(munmap, addr, length);
}