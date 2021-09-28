#pragma once

enum ptrace_req
{
    PTRACE_ATTACH,
    PTRACE_TRACEME,
    PTRACE_GETREGS,
    PTRACE_GETFREGS,
    PTRACE_CONT
};