#!/bin/sh
set -e
. ./build.sh
 
mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub
 
cp sysroot/boot/simple.kernel isodir/boot/simple.kernel
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "simple kernel" {
    multiboot /boot/simple.kernel
}
EOF
grub-mkrescue -o simple.iso isodir
