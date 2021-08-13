cmake_minimum_required(VERSION 3.10)

project(Chip-8_emulator VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

set(CMAKE_CXX_FLAGS "-O3 -D_REENTRANT -Wall -Wextra -Wpedantic")
add_definitions(-DNOT_CLANGD)

SET(CMAKE_EXPORT_COMPILE_COMMANDS true)

include_directories(/usr/include/SDL2)
link_libraries(SDL2 SDL2_ttf)

add_executable(chip8emu
    main.cpp
    Chip-8.h
    Chip-8.cpp
    sdl_file_chooser.h
    sdl_file_chooser.cpp
    sound.h
    sound.cpp
    fontset.h
    to_hex.h
    gfx.h
    license.h
    submodules/chip8asm/src/InputFile.cpp
    submodules/chip8asm/src/parser.cpp
    submodules/chip8asm/src/binary_generator.cpp
    submodules/chip8asm/src/Logger.cpp
)
