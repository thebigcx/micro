.global longjmp
longjmp:
    // Restore registers
    movq    0x0(%rdi), %rbx
    movq    0x8(%rdi), %r12
    movq    0x10(%rdi), %r13
    movq    0x18(%rdi), %r14
    movq    0x20(%rdi), %r15
    movq    0x28(%rdi), %rbp
    movq    0x30(%rdi), %rsp

    // Restore return address
    movq    0x38(%rdi), %rcx
    movq    %rcx, (%rsp)

    ret