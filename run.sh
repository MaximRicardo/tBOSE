#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

#qemu-system-x86_64 -drive format=raw,file=$SCRIPT_DIR/bin/os.bin,index=0,if=floppy -d int
qemu-system-x86_64 -drive format=raw,file=$SCRIPT_DIR/bin/os.bin,index=0,if=floppy -m 256M
