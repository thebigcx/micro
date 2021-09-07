qemu-system-x86_64 -serial stdio -m 512 -cpu qemu64 -bios /usr/share/ovmf/x64/OVMF.fd -cdrom dist/image.iso -d guest_errors --no-reboot --no-shutdown -s
