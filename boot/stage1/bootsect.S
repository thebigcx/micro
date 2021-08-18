.code16

    jmp     _start
    nop

.skip 0x3a

_start:
    mov     $0x42, %ah
    mov     $0x80, %dl
    mov     $dap, %si
    int     $0x13

loop:
    jmp     loop

.align 4
dap:
.byte 0x10
.byte 0
.word 1 // Number of sectors
.word 0x7e00 // Offset
.word 0 // Segment
.int 1 // LBA
.int 0


// read_sector(): read a sector by using BIOS int 13h
// in:  %cl = sector
//      %es:bx = buffer address
// out: %cl = next sector
//      %es:bx = buffer address + sector size
/*read_sector:
    mov     $0x2, %ah
    mov     $0x1, %al
    mov     $0x0, %ch
    mov     $0x0, %dh
    mov     $0x80, %dl
    int     $0x13
    ret*/
