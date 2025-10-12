cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

project(mypico C CXX ASM)
set(CMAKE_C_STANDARD 11)

pico_sdk_init()

add_executable(mypico
    main.c
    myos/myport.c
    myos/mytasks.c
    myos/systick.c
    myos/net_queue.c
    myos/safe_printf.c
)

target_include_directories(mypico PRIVATE
    myos
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(mypico
    pico_stdlib
    pico_cyw43_arch_lwip_poll
)

pico_enable_stdio_usb(mypico 0)
pico_enable_stdio_uart(mypico 1)

pico_add_extra_outputs(mypico)
