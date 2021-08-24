/*
 *  Tool to create initial ramdisks for Christian's kernel
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct fheader
{
    char name[128];
    uint64_t size;

} fheader_t;

int main(int argc, char** argv)
{
    FILE* initrd = fopen("initrd.img", "wb");

    for (int i = 0; i < argc / 2; i++)
    {
        char* src = argv[2 * i + 1];
        char* dst = argv[2 * i + 2];

        printf("Copying %s to /initrd/%s\n", src, dst);

        FILE* srcfile = fopen(src, "r");
        fseek(srcfile, 0, SEEK_END);
        size_t sz = ftell(srcfile);
        fseek(srcfile, 0, SEEK_SET);
        uint8_t* data = malloc(sz);
        fread(data, 1, sz, srcfile);
        fclose(srcfile);

        fheader_t hdr;
        memset(hdr.name, 0, sizeof(hdr.name));
        strcpy(hdr.name, dst);
        hdr.size = sz;
        fwrite(&hdr, 1, sizeof(fheader_t), initrd);
        fwrite(data, 1, sz, initrd);
    }

    fclose(initrd);

    return 0;
}
