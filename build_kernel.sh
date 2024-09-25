#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

nasm $SCRIPT_DIR/kernel/kernel_entry.s -f elf -o $SCRIPT_DIR/bin/kernel_entry.o

# Compile and link the kernel c code
make -C $SCRIPT_DIR

# i386-elf-ld -o $SCRIPT_DIR/bin/kernel.bin -Ttext 0x1000 $SCRIPT_DIR/bin/kernel_entry.o $SCRIPT_DIR/bin/kernel_c_code.o --oformat binary
ld -m elf_i386 -o $SCRIPT_DIR/bin/kernel.bin -Ttext 0x1000 $SCRIPT_DIR/bin/kernel_entry.o $SCRIPT_DIR/bin/kernel_c_code.o --oformat binary
