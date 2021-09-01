void _start(int argc, char** argv)
{
    asm ("int $0x80");
    for (;;);
}
