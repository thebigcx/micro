.code16

    jmp     _start
    nop

.skip 0x3a

_start:
    mov     $0x8000, %bx
    mov     $0x1, %cl
    call    read_sector
    jmp     _start

// read_sector(): read a sector by using BIOS int 13h
// in:  %cl = sector
//      %es:bx = buffer address
// out: %cl = next sector
//      %es:bx = buffer address + sector size
read_sector:
    mov     $0x2, %ah
    mov     $0x1, %al
    mov     $0x0, %ch
    mov     $0x0, %dh
    mov     $0x80, %dl
    int     $0x13
    ret
