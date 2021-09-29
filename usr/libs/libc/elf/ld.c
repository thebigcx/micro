#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <elf.h>
#include <sys/mman.h>

extern char** environ;

// TODO: use sysconf() to find this
#define PAGE_SIZE 0x1000

struct elf_info
{
    uintptr_t entry;
};

static void ref_resolve()
{
    printf("resolve reference\n");
}

// Open a shared object
void open_object(const char* path, struct elf_info* inf, uintptr_t base)
{
    printf("loading object %s...\n", path);

    // TODO: LD_LIBRARY_PATH environ var

    char full[256];
    strcpy(full, "/usr/lib/");
    strcat(full, path);

    int fd = open(path, O_RDONLY);
    if (fd != -1) goto found;

    fd = open(full, O_RDONLY);
    if (fd != -1) goto found;

    perror("ld.so: could not open object: ");
    exit(-1);

found:

    Elf64_Ehdr ehdr;
    read(fd, &ehdr, sizeof(Elf64_Ehdr));

    for (size_t i = 0; i < ehdr.e_phnum; i++)
    {
        Elf64_Phdr phdr;

        lseek(fd, ehdr.e_phoff + ehdr.e_phentsize * i, SEEK_SET);
        read(fd, &phdr, sizeof(Elf64_Phdr));

        if (phdr.p_type == PT_LOAD)
        {
            phdr.p_vaddr += base;

            void* addr = mmap(phdr.p_vaddr - (phdr.p_vaddr % PAGE_SIZE),
                        phdr.p_memsz + (PAGE_SIZE - (phdr.p_memsz % PAGE_SIZE)),
                        PROT_EXEC | PROT_READ | PROT_WRITE,
                        MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

            if (addr == MAP_FAILED)
            {
                perror("ld.so: failed to mmap");
                return -1;
            }
            
            memset((void*)phdr.p_vaddr, 0, phdr.p_memsz);

            lseek(fd, phdr.p_offset, SEEK_SET);
            read(fd, phdr.p_vaddr, phdr.p_filesz);
        }
    }

    for (size_t i = 0; i < ehdr.e_shnum; i++)
    {
        Elf64_Shdr shdr, strsect;

        lseek(fd, ehdr.e_shoff + ehdr.e_shentsize * i, SEEK_SET);
        read(fd, &shdr, sizeof(Elf64_Shdr));

        lseek(fd, ehdr.e_shoff + ehdr.e_shentsize * shdr.sh_link, SEEK_SET);
        read(fd, &strsect, sizeof(Elf64_Shdr));

        if (shdr.sh_type == SHT_DYNAMIC)
        {
            for (size_t j = 0; j < shdr.sh_size / sizeof(Elf64_Dyn); j++)
            {
                Elf64_Dyn dyn;

                lseek(fd, shdr.sh_offset + sizeof(Elf64_Dyn) * j, SEEK_SET);
                read(fd, &dyn, sizeof(Elf64_Dyn));

                if (dyn.d_tag == DT_NEEDED)
                {
                    // Get the library path
                    char lib[64];
                    lseek(fd, strsect.sh_offset + dyn.d_un.d_val, SEEK_SET);

                    size_t k = 0;
                    do
                    {
                        read(fd, &lib[k], 1);
                    } while (lib[k++] != 0);

                    struct elf_info info;
                    open_object(lib, &info, 0x10000000);
                }
            }
        }
    }

    inf->entry = ehdr.e_entry;

    close(fd);
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("usage: ld.so <executable>\n");
        return -1;
    }

    printf("[dynamically linking %s...]\n", argv[1]);

    struct elf_info info;
    open_object(argv[1], &info, 0);

    // Jump to the entry point
    void (*elf_start)(int, char**, char**) = info.entry;
    //elf_start(argc - 1, &argv[1], environ);

    return 0;
}