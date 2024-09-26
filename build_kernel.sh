#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

mkdir -p $SCRIPT_DIR/bin
mkdir -p $SCRIPT_DIR/kernel_bin

# Compile and link the kernel to an ELF file
make -C $SCRIPT_DIR

# The flat binary file is what will be loaded by the bootloader
# Therefore the ELF file has to be converted into a flat BIN file
objcopy -O binary $SCRIPT_DIR/kernel_bin/kernel.elf $SCRIPT_DIR/kernel_bin/kernel.bin
