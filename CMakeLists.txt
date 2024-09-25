cmake_minimum_required(VERSION 3.10)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/kernel_bin)

project(kernel)

set(CMAKE_C_FLAGS "-std=c99 -masm=intel -target i386-elf -Wall -Wextra -Wpedantic -pedantic -ffreestanding -m32 -fno-stack-protector -nostdlib")

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
