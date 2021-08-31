#pragma once

#include <types.h>
#include <platform.h>

FORCE_INLINE void outb(uint16_t port, uint8_t val)
{
    asm volatile ("outb %0, %1" :: "a"(val), "Nd"(port));
}

FORCE_INLINE void outw(uint16_t port, uint16_t val)
{
    asm volatile ("outw %0, %1" :: "a"(val), "Nd"(port));
}

FORCE_INLINE void outl(uint16_t port, uint32_t val)
{
    asm volatile ("outl %0, %1" :: "a"(val), "Nd"(port));
}

FORCE_INLINE uint8_t inb(uint16_t port)
{
    uint8_t v;
    asm volatile ("inb %1, %0" : "=a"(v) : "Nd"(port));
    return v;
}

FORCE_INLINE uint16_t inw(uint16_t port)
{
    uint16_t v;
    asm volatile ("inw %1, %0" : "=a"(v) : "Nd"(port));
    return v;
}

FORCE_INLINE uint32_t inl(uint16_t port)
{
    uint32_t v;
    asm volatile ("inl %1, %0" : "=a"(v) : "Nd"(port));
    return v;
}