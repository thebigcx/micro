#pragma once

#define FORCE_INLINE __attribute__((always_inline)) inline
#define PACKED       __attribute__((packed))
#define ALIGN(a)     __attribute__((aligned(a)))
#define SECTION(s)   __attribute__((section(s)))
#define USED         __attribute__((used))