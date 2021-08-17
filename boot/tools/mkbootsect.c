#include <stdio.h>
#include <assert.h>

int main(int argc, char** argv)
{
    assert(argc == 2);

    system("dd if=/dev/zero of=boot.img bs=1024 count=10000");
    system("mkfs.vfat -F16 boot.img");

    FILE* in = fopen(argv[1], "r");
    FILE* out = fopen("boot.img", "r+");

    char jmp[3];
    fread(jmp, 3, 1, in);
    fwrite(jmp, 3, 1, out);

    fseek(in, 0x3c, SEEK_SET);
    fseek(out, 0x3c, SEEK_SET);

    char code[448];
    fread(code, 448, 1, in);
    fwrite(code, 448, 1, out);

    fclose(out);
    fclose(in);

    return 0;
}
