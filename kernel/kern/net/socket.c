#include <micro/sys.h>

SYSCALL_DEFINE(socket, int domain, int type, int protocol)
{
    return 0;
}