dd if=/dev/zero of=initrd.img bs=1k count=14400
chown "$SUDO_UID":"$SUDO_GID" initrd.img
mkfs.fat -F32 initrd.img 14400
mkdir -p mnt
sudo mount initrd.img mnt
sudo cp ../usr/srv/init/init mnt/
sudo cp -r ../root/* mnt/
sudo umount mnt
rm -rf mnt
