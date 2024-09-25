#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

mkdir -p $SCRIPT_DIR/bin

nasm $SCRIPT_DIR/boot/boot.s -f bin -o $SCRIPT_DIR/bin/boot.bin
nasm $SCRIPT_DIR/boot/second_stage.s -f bin -o $SCRIPT_DIR/bin/second_stage.bin

cat $SCRIPT_DIR/bin/boot.bin $SCRIPT_DIR/bin/second_stage.bin > $SCRIPT_DIR/bin/boot_loader.bin

# Build the kernel aswell
. $SCRIPT_DIR/build_kernel.sh

cat $SCRIPT_DIR/bin/boot_loader.bin $SCRIPT_DIR/kernel_bin/kernel.bin > $SCRIPT_DIR/bin/os.bin
