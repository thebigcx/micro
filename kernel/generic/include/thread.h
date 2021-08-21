#pragma once

#include <cpu.h>

struct thread
{
    int id;
    struct regs regs;
};
