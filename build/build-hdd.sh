dd if=/dev/zero of=dist/image.hdd bs=1M count=0 seek=64

parted -s dist/image.hdd mklabel gpt

parted -s dist/image.hdd mkpart ESP fat32 2048s 100%
parted -s dist/image.hdd set 1 esp on

if [ ! -d ./limine ]; then
    git clone https://github.com/limine-bootloader/limine.git --branch=v2.0-branch-binary --depth=1
    make -C limine
fi

./limine/limine-install dist/image.hdd

USED_LOOPBACK=$(sudo losetup -Pf --show dist/image.hdd)
 
# Format the ESP partition as FAT32.
sudo mkfs.fat -F 32 ${USED_LOOPBACK}p1
 
# Mount the partition itself.
mkdir -p imgmnt
sudo mount ${USED_LOOPBACK}p1 imgmnt
 
# Copy the relevant files over.
sudo mkdir -p imgmnt/EFI/BOOT
sudo cp -v ../kernel/dist/vmkernel initrd.img limine.cfg limine/limine.sys imgmnt/
sudo cp -v limine/BOOTX64.EFI imgmnt/EFI/BOOT/
 
# Sync system cache and unmount partition and loopback device.
sync
sudo umount imgmnt
sudo losetup -d ${USED_LOOPBACK}
