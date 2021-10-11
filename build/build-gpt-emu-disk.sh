dd if=/dev/zero of=dist/disk.img bs=1k count=144000

parted -s dist/disk.img mklabel gpt

parted -s dist/disk.img mkpart FS ext2 2048s 100%

USED_LOOPBACK=$(sudo losetup -Pf --show dist/disk.img)

sudo mkfs.ext2 ${USED_LOOPBACK}p1

# Mount the partition itself.
mkdir -p mnt
sudo mount ${USED_LOOPBACK}p1 mnt

sudo cp -r ../root/* mnt/
sudo chown -R 1000:1000 mnt/

sudo umount mnt
rmdir mnt
