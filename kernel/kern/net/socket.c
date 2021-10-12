#include <micro/sys.h>

SYSCALL_DEFINE(socket, int domain, int type, int protocol)
{
    return 0;
}

SYSCALL_DEFINE(bind, int sockfd, const struct sockaddr* addr,
               socklen_t addrlen)
{
    return 0;
}

SYSCALL_DEFINE(accept, int sockfd, struct sockaddr* addr, socklen_t* addrlen)
{
    return 0;   
}
