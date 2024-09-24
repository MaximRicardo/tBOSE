#!/bin/bash

nasm boot/boot.s -f bin -o bin/boot.bin
nasm boot/second_stage.s -f bin -o bin/second_stage.bin

cat bin/boot.bin bin/second_stage.bin > bin/boot_loader.bin
