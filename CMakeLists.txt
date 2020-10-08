cmake_minimum_required(VERSION 3.10)

project(Chip-8_emulator VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

set(CMAKE_CXX_FLAGS "-O3 -D_REENTRANT")

include_directories(/usr/include/SDL2)
link_libraries(SDL2 SDL2_ttf ncurses)

add_executable(chip8emu
    Chip-8.cpp  DoubleAsker.cpp  main.cpp  sdl_file_chooser.cpp  sound.cpp
    Chip-8.h  DoubleAsker.h  fontset.h  sdl_file_chooser.h  sound.h  to_hex.h
)
