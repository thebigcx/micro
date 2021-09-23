cd ../usr
make -I~/src/micro/share
make install DESTDIR=~/src/micro/root -I~/src/micro/share
cd ../kernel
make -I~/src/micro/share
make install DESTDIR=~/src/micro/root -I~/src/micro/share
cd ../build
sh build-initrd.sh
sh build-emu-disk.sh
sh build-limine.sh
