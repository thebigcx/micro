.global enable_sse
enable_sse:
    movq    %cr0, %rax
    andw    $0xfffb, %ax
    orw     $0x2, %ax
    movq    %rax, %cr0

    movq    %cr4, %rax
    orq     $(3 << 9), %rax
    movq    %rax, %cr4

    ret

.global enable_avx
enable_avx:
    pushq   %rax
    pushq   %rcx
    pushq   %rdx

    xorq    %rcx, %rcx
    xgetbv              // Get XCR0
    orq     $0x7, %rax
    xsetbv              // Set XCR0

    popq    %rdx
    popq    %rcx
    popq    %rax
    ret