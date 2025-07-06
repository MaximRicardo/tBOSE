#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

mkdir -p $SCRIPT_DIR/bin

nasm $SCRIPT_DIR/boot/boot.s -f bin -o $SCRIPT_DIR/bin/boot.bin
nasm $SCRIPT_DIR/boot/second_stage.s -f bin -o $SCRIPT_DIR/bin/second_stage.bin

cat $SCRIPT_DIR/bin/boot.bin $SCRIPT_DIR/bin/second_stage.bin > $SCRIPT_DIR/bin/boot_loader.bin

# Build the kernel aswell
. $SCRIPT_DIR/build_kernel.sh

cat $SCRIPT_DIR/bin/boot_loader.bin $SCRIPT_DIR/kernel_bin/kernel.bin "test_str" > $SCRIPT_DIR/bin/os.bin

: '
dd if=/dev/zero of=$SCRIPT_DIR/floppy.img bs=1024 count=1440
dd if=$SCRIPT_DIR/bin/os.bin of=$SCRIPT_DIR/floppy.img seek=0 conv=notrunc

echo "1"
mkdir -p $SCRIPT_DIR/iso/
echo "2"
cp $SCRIPT_DIR/floppy.img $SCRIPT_DIR/iso/
echo "3"
#genisoimage -quiet -V 'OS' -input-charset iso8859-1 -o $SCRIPT_DIR/os.iso -b floppy.img \
#    -hide $SCRIPT_DIR/floppy.img $SCRIPT_DIR/iso/
genisoimage -quiet -o $SCRIPT_DIR/os.iso $SCRIPT_DIR/bin/os.bin
echo "4"

mv $SCRIPT_DIR/os.iso $SCRIPT_DIR/iso/
rm $SCRIPT_DIR/floppy.img
'
