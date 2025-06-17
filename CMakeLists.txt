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
)

target_include_directories(mypico PRIVATE
    myos
)

target_link_libraries(mypico pico_stdlib)

pico_add_extra_outputs(mypico)
