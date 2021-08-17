.code16

    jmp _start
    nop

.skip 0x3a

_start:
    mov     $0x0e, %ah
    mov     $'X', %al
    int     $0x10


    //cli

loop:
    jmp     loop
