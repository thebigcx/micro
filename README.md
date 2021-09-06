# Chistian's Operating System

Includes the kernel, userspace and bootloader.

# Building

```bash
make -C kernel
cd build
sh build-initrd.sh
sh build-limine.sh
```

Run ```make``` in the /kernel directory. This should produce the file ```kernel/dist/vmkernel```, which can be run by any Stivale2-compliant bootloader.
Run ```sh build-initrd``` in /build to generate an ```initrd.img``` to pass to the bootloader.

# Kernel Features
- Higher half kernel
- Amd64 architecture
- Virtual File System
- FAT32 filesystem driver
- Multithreading with a pre-emptive scheduler
- Syscalls
- ELF loading
- Module loading

# Userland Features
- Bare-bones libc
- Basic shell
- Basic coreutils (cat, ls, pwd, etc)
