dd if=/dev/zero of=initrd.img bs=1k count=14400
chown "$SUDO_UID":"$SUDO_GID" initrd.img
mkfs.fat -F32 initrd.img 14400
#mkfs.ext2 initrd.img 1440
mkdir -p mnt
sudo mount initrd.img mnt
sudo cp ../usr/srv/init/init mnt/
sudo cp -r ../root/* mnt/
sudo cp ../usr/apps/sh/sh mnt/usr/bin/sh
sudo cp -r ../kernel/include/uapi/micro/* mnt/usr/include/micro/
#sudo cp -r ../root/* mnt/
sudo umount mnt
rm -rf mnt
