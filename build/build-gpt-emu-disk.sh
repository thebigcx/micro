dd if=/dev/zero of=dist/disk.img bs=1k count=144000

parted -s dist/disk.img mklabel gpt

parted -s dist/disk.img mkpart ESP fat32 0% 50%
parted -s dist/disk.img set 1 esp on
parted -s dist/disk.img mkpart Root ext2 50% 100%

./limine/limine-install dist/disk.img 1

USED_LOOPBACK=$(sudo losetup -Pf --show dist/disk.img)

# Format the ESP partition as FAT32.
sudo mkfs.fat -F 32 ${USED_LOOPBACK}p1
 
# Mount the partition itself.
mkdir -p mnt
sudo mount ${USED_LOOPBACK}p1 mnt
 
# Copy the relevant files over.
sudo mkdir -p mnt/EFI/BOOT
sudo cp -v ../kernel/dist/vmkernel initrd.img limine.cfg limine/limine.sys mnt/
sudo cp -v limine/BOOTX64.EFI mnt/EFI/BOOT/

sync
sudo umount mnt

sudo mkfs.ext2 ${USED_LOOPBACK}p2

# Mount the partition itself.
sudo mount ${USED_LOOPBACK}p2 mnt

sudo cp -r ../root/* mnt/
sudo chown -R 1000:1000 mnt/

sudo umount mnt
rmdir mnt

