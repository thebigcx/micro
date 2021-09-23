# Chistian's Operating System

Includes the kernel and userspace libraries/applications.

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
- Ext2 filesystem driver
- Multithreading with a pre-emptive scheduler
- User mode (ring 3)
- Syscalls
- ELF loading
- Module loading
- DevFS filesystem
- AHCI disk driver
- RTC driver
- PS/2 keyboard driver
- UNIX Signals

# Userland Features
- Respectable libc
- Basic shell
- Coreutils (ls, pwd, cat, date, rm, mkdir, ...)
- Work-in-progress ports - nasm, GNU binutils, and Lua
