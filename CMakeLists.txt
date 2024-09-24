cmake_minimum_required(VERSION 3.10)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

project(kernel)

add_compile_options(-std=c99 -target i386-elf -mgeneral-regs-only -Wall -Wextra -Wpedantic -pedantic -ffreestanding -m32)

# Generate a relocatable object file
add_link_options(-target i386-elf -r)

file(GLOB_RECURSE Sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/kernel/*.c")

add_executable(kernel "${Sources}")

set_target_properties(
    kernel
    PROPERTIES
        OUTPUT_NAME "kernel_c_code"
        SUFFIX ".o"
)
