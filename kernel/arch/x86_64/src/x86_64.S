/*
 *  GDT flushing assembly
 */

.global lgdt_full

// void lgdt_full(uint16_t, uint16_t)
lgdt_full:
    lgdt    (%rdi)
    mov     %dx, %ax  // Data
    mov     %ax, %ds
    mov     %ax, %es
    mov     %ax, %fs
    mov     %ax, %gs
    mov     %ax, %ss
    mov     %rsi, %rax // Code
    pushq   %rax
    pushq   %rsi
    retfq