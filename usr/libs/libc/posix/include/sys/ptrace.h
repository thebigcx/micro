#pragma once

#include <sys/types.h>

enum __ptrace_request
{
    PTRACE_ATTACH,
    PTRACE_TRACEME,
    PTRACE_GETREGS,
    PTRACE_GETFREGS,
};

long ptrace(enum __ptrace_request req, pid_t pid, void* addr, void* data);