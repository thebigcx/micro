qemu-system-x86_64 -serial stdio -m 512 -cpu qemu64 -bios /usr/share/ovmf/x64/OVMF.fd -drive file=dist/image.hdd,if=ide -d guest_errors --no-reboot --no-shutdown -s --enable-kvm 
