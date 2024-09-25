#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

mkdir -p bin
mkdir -p kernel_bin

nasm $SCRIPT_DIR/kernel/kernel_entry.s -f elf -o $SCRIPT_DIR/kernel_bin/kernel_entry.o
nasm $SCRIPT_DIR/kernel/interrupts.s -f elf -o $SCRIPT_DIR/kernel_bin/interrupts.o

# Compile and link the kernel c code
make -C $SCRIPT_DIR

ld -m elf_i386 -o $SCRIPT_DIR/kernel_bin/kernel.bin -Ttext 0x1000 $SCRIPT_DIR/kernel_bin/kernel_entry.o $SCRIPT_DIR/kernel_bin/kernel_c_code.o $SCRIPT_DIR/kernel_bin/interrupts.o --oformat binary
