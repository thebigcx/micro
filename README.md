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
