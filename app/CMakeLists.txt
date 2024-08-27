cmake_minimum_required(VERSION 3.24.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(
    app

    DESCRIPTION 
        "Elerium Cold Wallet"
    LANGUAGES 
        C
    VERSION 
        0.0.1
)

target_include_directories(
    app

    PRIVATE
        ${CMAKE_BINARY_DIR}/app/include
)

target_sources(
    app
    
    PRIVATE
        src/main.c
)

target_compile_options(
    app

    PRIVATE
        -Wall
)



