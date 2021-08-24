OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

SECTIONS
{
    .text :
    {
        *(.*)
    }
    .data :
    {
        *(.data)
    }
    .bss :
    {
        *(.bss)
    }
}
