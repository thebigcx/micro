if [ ! -d ./limine ]; then
        git clone https://github.com/limine-bootloader/limine.git --branch=v2.0-branch-binary --depth=1
        make -C limine
fi

mkdir -p iso
cp -v ../kernel/dist/vmkernel initrd.img limine.cfg limine/limine.sys limine/limine-cd.bin limine/limine-eltorito-efi.bin iso
mkdir -p dist
xorriso -as mkisofs -b limine-cd.bin \
        -no-emul-boot -boot-load-size 4 -boot-info-table \
        --efi-boot limine-eltorito-efi.bin \
        -efi-boot-part --efi-boot-image --protective-msdos-label \
        iso -o dist/image.iso

./limine/limine-install dist/image.iso
qemu-system-x86_64                              \
    dist/image.iso                              \
    -serial stdio				\
    -m 512                                      \
    -smp 1                                      \
    -s -d guest_errors                          \
    -no-reboot -no-shutdown                     \
    -drive id=disk,file=dist/disk.img,if=none   \
    -device ahci,id=ahci                        \
    -device ide-hd,drive=disk,bus=ahci.0        \
    -rtc base=localtime                         \
    --enable-kvm
