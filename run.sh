#!/bin/bash

qemu-system-x86_64 -drive format=raw,file=bin/boot_loader.bin,index=0,if=floppy
