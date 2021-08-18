#pragma once

struct cpu_info
{
    struct tss tss;
    union gdtent gdt[7];
};
