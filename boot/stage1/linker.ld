OUTPUT_FORMAT("binary")
SECTIONS
{
    . = 0x7c00;
    .text :
    {
        *(.*)
        . = 0x1fe;
        SHORT(0xaa55)
    }
}
