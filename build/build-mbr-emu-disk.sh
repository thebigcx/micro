dd if=/dev/zero of=dist/mbr.img bs=1k count=144000

parted -s dist/mbr.img mklabel msdos

parted -s dist/mbr.img mkpart primary 10% 100%

USED_LOOPBACK=$(sudo losetup -Pf --show dist/mbr.img)

sudo mkfs.ext2 ${USED_LOOPBACK}p1

mkdir -p mnt
sudo mount ${USED_LOOPBACK}p1 mnt

sudo cp -r ../root/* mnt/
sudo chown -R 1000:1000 mnt/

sudo mkdir -p mnt/boot
sudo cp -v ../kernel/dist/vmkernel initrd.img limine.cfg limine/limine.sys mnt/boot

sync
sudo umount mnt

# Format the ESP partition as FAT32.
#sudo mkfs.ext2 ${USED_LOOPBACK}p2

# Mount the partition itself.
#sudo mount ${USED_LOOPBACK}p2 mnt

#sudo cp -r ../root/* mnt/
#sudo chown -R 1000:1000 mnt/

#sync
#sudo umount mnt
rmdir mnt

./limine/limine-install dist/mbr.img
