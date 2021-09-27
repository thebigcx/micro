.global setjmp
setjmp:
    // Save registers
    movq    %rbx, 0x0(%rdi)
    movq    %r12, 0x8(%rdi)
    movq    %r13, 0x10(%rdi)
    movq    %r14, 0x18(%rdi)
    movq    %r15, 0x20(%rdi)
    movq    %rbp, 0x28(%rdi)
    movq    %rsp, 0x30(%rdi)

    // Save return address
    movq    (%rsp), %rax
    movq    %rax, 0x38(%rdi)
    xorq    %rax, %rax
    ret