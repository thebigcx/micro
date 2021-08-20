/*
 *  GDT and TSS flushing assembly
 */

.global rel_segs
.global lgdt
.global lidt
.global ltr

// void rel_segs(uint16_t, uint16_t)
rel_segs:
    mov     %si, %ax  // Data
    mov     %ax, %ds
    mov     %ax, %es
    mov     %ax, %fs
    mov     %ax, %gs
    mov     %ax, %ss
    mov     %rdi, %rax // Code
    pushq   %rax
    pushq   %rdi
    retfq

// void lgdt(struct descptr*)
lgdt:
    lgdt    (%rdi)
    ret

lidt:
    lidt    (%rdi)
    ret

ltr:
    mov     %rdi, %rax
    ltr     %ax
    ret
