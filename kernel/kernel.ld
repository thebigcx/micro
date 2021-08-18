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
        *(.text)
    }

    .rodata : AT(ADDR(.rodata) - VIRTADDR)
    {
        *(.rodata)
    }

    .data : AT(ADDR(.data) - VIRTADDR)
    {
        *(.data)
    }

    .bss : AT(ADDR(.bss) - VIRTADDR)
    {
        *(COMMON)
        *(.bss)
    }

    _kernel_end = .;
}
