#!/bin/bash

qemu-system-x86_64 -drive format=raw,file=bin/boot.bin,index=0,if=floppy
