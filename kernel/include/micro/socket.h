#pragma once

#include <micro/types.h>

struct sockaddr
{
    sa_family_t sa_family;
    char        sa_data[14];
};

struct socket;

struct sock_ops
{
    int (*bind)(struct socket*, const struct sockaddr*, socklen_t);
    struct socket* (*accept)(struct socket*, struct sockaddr*, socklen_t*);
};

struct socket
{
    struct sock_ops ops;
};


