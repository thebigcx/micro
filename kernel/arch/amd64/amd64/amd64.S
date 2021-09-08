/*
 *  GDT flushing assembly
 */

.global lgdt_full

// void lgdt_full(struct gdtent*, uint16_t, uint16_t)
lgdt_full:
    lgdt    (%rdi)
    mov     %dx, %ax  // Data
    mov     %ax, %ds
    mov     %ax, %es
    mov     %ax, %fs
    mov     %ax, %gs
    mov     %ax, %ss

    // Reload the code segment
    //popq    %rax // Pop the previous return address

    movq    %rsp, %rbp

    
    //pushq   %rax
    //pushq   %rax
    leaq    .done(%rip), %rax
    
    pushq   %rdx // SS
    pushq   %rbp // RSP
    pushfq       // RFLAGS
    pushq   %rsi // CS
    pushq   %rax // RIP
    iretq

.done:
    ret