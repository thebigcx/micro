dd if=/dev/zero of=dist/disk.img bs=1k count=14400
#mkfs.fat -F32 dist/disk.img
#mkfs.vfat -F32 dist/disk.img
mkfs.ext2 dist/disk.img
mkdir -p mnt
sudo mount dist/disk.img mnt
sudo cp -r ../root/* mnt/
sudo chown -R 1000:1000 mnt/
sudo umount mnt
rmdir mnt