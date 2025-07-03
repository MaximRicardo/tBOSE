#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

#qemu-system-x86_64 -drive format=raw,file=$SCRIPT_DIR/bin/os.bin,index=0,if=floppy -d int
#qemu-system-x86_64 -drive format=raw,file=$SCRIPT_DIR/bin/os.bin,index=0,if=floppy -m 256M
#qemu-system-x86_64 -drive format=raw,file=$SCRIPT_DIR/iso/os.iso,if=none -m 256M
#qemu-system-x86_64 -drive file=$SCRIPT_DIR/iso/os.iso,if=floppy -m 256M
#qemu-system-x86_64 -boot d -drive format=raw,file=$SCRIPT_DIR/iso/floppy.img,if=floppy -m 256M
#qemu-system-x86_64 -boot d -drive format=raw,file=$SCRIPT_DIR/bin/os.bin,index=0,if=floppy -m 256M
#qemu-system-x86_64 -cdrom $SCRIPT_DIR/iso/os.iso

qemu-system-x86_64 -drive format=raw,file=$SCRIPT_DIR/bin/os.bin
