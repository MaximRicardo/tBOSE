#!/bin/bash

nasm kernel/kernel_entry.s -f elf -o bin/kernel_entry.o

i386-elf-gcc -std=c99 -mgeneral-regs-only -Wall -Wextra -Wpedantic -pedantic -ffreestanding -m32 -c kernel/kernel_main.c -o bin/kernel_main.o

i386-elf-ld -o bin/kernel.bin -Ttext 0x1000 bin/kernel_entry.o bin/kernel_main.o --oformat binary
