#!/bin/sh
set -e
. ./iso.sh
 
qemu-system-$(./target-triplet-to-arch.sh $HOST) -cdrom simple.iso -d int -monitor stdio -D ./qemu-log.txt -accel tcg
