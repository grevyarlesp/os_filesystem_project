cmake_minimum_required(VERSION 3.19)
project(Filesystem_Viewer)

set(CMAKE_CXX_STANDARD 14)

add_executable(Filesystem_Viewer main.cpp Filesystem_Reader.cpp Filesystem_Reader.h FAT32_Reader.cpp FAT32_Reader.h Item.cpp Item.h)

set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")