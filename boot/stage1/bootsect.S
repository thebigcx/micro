.code16

    jmp     _start
    nop

.skip 0x3a

_start:
    cli
    cld
    ljmp    $0x0, $1f // Initialize CS
1:

    xor     %ax, %ax
    mov     %ax, %ds
    mov     %ax, %es
    mov     %ax, %ss
    mov     $0x7c00, %sp

    sti
    
    mov     $0x42, %ah
    mov     $0x80, %dl
    mov     $dap, %si
    int     $0x13

    movw    %cs, %ax
    movw    %ax, %ds

    //movl    $gdt32, %eax
    //movl    %eax, (gdtptr32)

    lgdtl   (gdtptr32)

    movl    %cr0, %eax
    orl     $1, %eax
    movl    %eax, %cr0

    ljmpl   $0x8, $(entry32)

    hlt

.align 4
dap:
.byte 0x10
.byte 0
.word 20 // Number of sectors
offset:
.word 0x7e00 // Offset
.word 0 // Segment
.int 1 // LBA
.int 0

.align 16
gdt32:
    // Null
    .8byte 0x0

    .4byte 0x0000ffff 
    .4byte 0x00cf9a00

    .4byte 0x0000ffff
    .4byte 0x00cf9200
    
gdt32_end:

gdtptr32:
    .2byte gdt32_end - gdt32 - 1
    .4byte gdt32

.code32
entry32:
    cli
    
    movw    $0x10, %ax
    movw    %ax, %ds
    movw    %ax, %ss
    movw    %ax, %es
    movw    %ax, %fs
    movw    %ax, %gs

    mov     $0x2f4b2f4f, %eax
    mov     %eax, 0xb8000

    mov     $0x1000, %edi
    mov     %edi, %cr3
    xor     %eax, %eax
    rep stosl
    mov     %cr3, %edi

    movl    $0x2003, (%edi) // PDPT, Present, Writable
    add     $0x1000, %edi
    movl    $0x3003, (%edi) // PD, Present, Writable
    add     $0x1000, %edi
    movl    $0x4003, (%edi) // PT, Present, Writable
    add     $0x1000, %edi

    mov     $0x00000003, %ebx
    mov     $512, %ecx

.set_entry:
    movl    %ebx, (%edi)
    add     $0x1000, %ebx
    add     $0x8, %edi
    loop    .set_entry

    // Enable paging (PAE)
    mov     %cr4, %eax
    or      $(1 << 5), %eax
    mov     %eax, %cr4

    // Set the Model Specific Register
    movl    $0xc0000080, %ecx
    rdmsr
    orl     $0x800, %eax
    orl     $(1 << 8), %eax
    wrmsr

    // Set Control Register 0
    movl    %cr0, %eax
    orl     $0x80000000, %eax
    movl    %eax, %cr0

    mov     $0x2f4b2f4f, %eax
    mov     %eax, 0xb8004

    lgdtl   (gdtptr64)

    ljmpl   $0x8, $(entry64)

.code64
entry64:
    hlt

.align 16
gdt64:
    // Null
    .8byte 0x0

    .4byte 0x00000000
    .4byte 0x00209800

    .4byte 0x00000000
    .4byte 0x00009200
gdt64_end:

gdtptr64:
    .2byte gdt64_end - gdt64 - 1
    .8byte gdt64