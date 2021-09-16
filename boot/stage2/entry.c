void main()
{
    *((unsigned int*)0xb8004) = 0x2f4b2f4f;
    asm ("hlt");
}