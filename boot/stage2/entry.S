    mov     $0xe, %ah
    mov     $'x', %al
    int     $0x10

loop:
    jmp loop
