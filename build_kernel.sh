#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

mkdir -p $SCRIPT_DIR/bin
mkdir -p $SCRIPT_DIR/kernel_bin

# Compile and link the kernel to an ELF file
make -C $SCRIPT_DIR

# The flat binary file is what will be loaded by the bootloader
# Therefore the ELF file has to be converted into a flat BIN file
objcopy -O binary $SCRIPT_DIR/kernel_bin/kernel.elf $SCRIPT_DIR/kernel_bin/kernel.bin

KERNEL_BIN_SIZE=$(stat -c%s "$SCRIPT_DIR/kernel_bin/kernel.bin")
KERNEL_BIN_RESIZED=$((512*40))

echo "Kernel bin is $KERNEL_BIN_SIZE bytes."

if [[ $KERNEL_BIN_SIZE -gt $KERNEL_BIN_RESIZED ]]; then
    echo "Kernel bin is too large!"
elif [[ $KERNEL_BIN_SIZE -lt $KERNEL_BIN_RESIZED ]]; then
    truncate -s $KERNEL_BIN_RESIZED $SCRIPT_DIR/kernel_bin/kernel.bin
fi
