#pragma once

#include <cpu.h>

struct thread
{
    unsigned int id;
    struct regs regs;
};
