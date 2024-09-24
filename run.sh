#!/bin/bash

qemu-system-x86_64 -drive format=raw,file=bin/os.bin,index=0,if=floppy
