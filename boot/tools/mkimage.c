#include <stdio.h>
#include <assert.h>

int main(int argc, char** argv)
{
    system("dd if=/dev/zero of=boot.img bs=1024 count=10000");
    //system("mkfs.vfat -F16 boot.img");

    FILE* bootsect = fopen(argv[1], "r");
    FILE* out = fopen("boot.img", "r+");
    FILE* loader = fopen("stage2/loader", "r");

    char jmp[3];
    fread(jmp, 3, 1, bootsect);
    fwrite(jmp, 3, 1, out);

    fseek(bootsect, 0x3c, SEEK_SET);
    fseek(out, 0x3c, SEEK_SET);

    char code[452];
    fread(code, 452, 1, bootsect);
    fwrite(code, 452, 1, out);

    printf("%d\n", ftell(out));

    // Write the rest of the bootloader
    fseek(loader, 0, SEEK_END);
    size_t loader_size = ftell(loader);
    fseek(loader, 0, SEEK_SET);

    char* loader_data = malloc(loader_size);
    fread(loader_data, loader_size, 1, loader);

    printf("%d\n", ftell(bootsect));
    fwrite(loader_data, loader_size, 1, out);

    fclose(out);
    fclose(bootsect);
    fclose(loader);
    return 0;
}
