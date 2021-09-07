OUTPUT_FORMAT(elf64-x86-64)

ENTRY(kmain_st2)

SECTIONS
{
	VIRTADDR = 0xffffffff80000000;

    . = VIRTADDR;
    . += 0x200000;

    _kernel_start = .;

    .stivale2hdr : AT(ADDR(.stivale2hdr) - VIRTADDR)
    {
        KEEP(*(.stivale2hdr))
    }

    .text : AT(ADDR(.text) - VIRTADDR)
    {
        *(.text*)
        . = ALIGN(0x1000);
    }

    . += 0x1000;

    .rodata : AT(ADDR(.rodata) - VIRTADDR)
    {
        *(.rodata*)
        . = ALIGN(0x1000);
    }

    . += 0x1000;

    .data : AT(ADDR(.data) - VIRTADDR)
    {
        *(.data*)
        . = ALIGN(0x1000);
    }

    .dynamic :
    {
        *(.dynamic)
        . = ALIGN(0x1000);
    }

    .bss : AT(ADDR(.bss) - VIRTADDR)
    {
        *(COMMON)
        *(.bss*)
        . = ALIGN(0x1000);
    }

    _kernel_end = .;
}
