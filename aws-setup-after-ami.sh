#! /bin/bash

sudo apt update
sudo apt install git gcc g++ flex bison build-essential fakeroot libncurses5-dev libssl-dev ccache flex bison libelf-dev dwarves busybox qemu-system-x86 patchelf cmake libevent-dev

cmake -DLINUX_DIR=../linux-dev/src -DBUILD_TESTING=OFF -DLANG=CXX -DCOMPILER=g++ -DWITH_MATH=1 -DWITH_THREAD=1 -DWITH_VDSO=1 -S user-dev/ -B user-dev/build

(cd user-dev/build; make)

(cd user-dev/build; make install)

echo "Setup complete. Now run the following to test if this works"
echo ""

echo "cd linux-dev/src"
echo "cp ../scripts/qemu-script.sh ."
echo "./qemu-script.sh"

echo ""
echo "If running qemu succeeds we are good to go. If you want to test the application benchmarks you could do:"

echo ""

echo "./../../glibc-dev/scripts/use-this-glibc.py glibc-install/ bench-install/bin/memcached /lib/x86_64-linux-gnu/libevent-2.1.so.7"
echo "LD_PRELOAD=./interpose-libs/libpthread-mutex-profile.so ./bench-install/bin/memcached -u noah -t 8 -m 4096 -n 2048 &"

echo "./bench-install/bin/memaslap -s 127.0.0.1:11211 -S 1s -B -T 8 -c 32"

echo ""

echo "If this runs until OOM (or manual shutdown) we are good. If you see it OOM in < 1minute we need a bigger instance."







