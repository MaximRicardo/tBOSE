cmake_minimum_required(VERSION 3.10)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/kernel_bin)

project(kernel)

set(CMAKE_C_FLAGS "-std=c99 -target i386-elf -Wall -Wextra -Wpedantic -pedantic -ffreestanding -m32 -fno-stack-protector -nostdlib -ffunction-sections -mcmodel=kernel -Og -ggdb3")
set(CMAKE_EXE_LINKER_FLAGS "-target i386-elf -T${CMAKE_CURRENT_SOURCE_DIR}/linker.ld -Ttext 0xc0001000 -z noexecstack")
set(CMAKE_ASM_NASM_OBJECT_FORMAT "elf")

enable_language(ASM_NASM)

file(GLOB_RECURSE kernel_c_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/kernel/*.c")
file(GLOB_RECURSE kernel_asm_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/kernel/*.s")

foreach(X IN ITEMS ${kernel_asm_sources})
    set_source_files_properties(${X} PROPERTIES LANGUAGE ASM_NASM)
endforeach()

add_executable(kernel ${kernel_c_sources} ${kernel_asm_sources})

set_target_properties(
    kernel
    PROPERTIES
        OUTPUT_NAME "kernel"
        SUFFIX ".elf"
)
