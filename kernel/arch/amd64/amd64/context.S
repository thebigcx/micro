/*
 * Switch context of CPU
 */

.macro popaq

    pop     %r15
    pop     %r14
    pop     %r13
    pop     %r12
    pop     %r11
    pop     %r10
    pop     %r9
    pop     %r8
    pop     %rbp
    pop     %rdi
    pop     %rsi
    pop     %rdx
    pop     %rcx
    pop     %rbx

.endm

// void _switch_ctx(RegisterCtx*, uintptr_t, uint16_t)
.global _switch_ctx
_switch_ctx:
    mov     %dx, %ax
    mov     %ax, %ds
    mov     %ax, %es
    mov     %ax, %fs
    mov     %ax, %gs

    movq    %rdi, %rsp
    movq    %rsi, %rax
    popaq

    movq    %rax, %cr3

    popq    %rax
    // Stack contains RSP, CS, RIP, RFLAGS, SS
    iretq

    ret // Return from signal handler (not used on a regular context switch)