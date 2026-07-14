cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico_w)
include("$ENV{PICO_SDK_PATH}/pico_sdk_init.cmake")

project(rp2040_fib66 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set (CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(rp2040_fib66 
    main.c
)

target_link_libraries(rp2040_fib66
    pico_stdlib
)

pico_enable_stdio_usb(rp2040_fib66 1)
pico_enable_stdio_uart(rp2040_fib66 0)

pico_add_extra_outputs(rp2040_fib66)
