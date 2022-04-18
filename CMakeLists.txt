cmake_minimum_required(VERSION 3.20)
project(archiver)

set(CMAKE_CXX_STANDARD 20)

add_executable(archiver main.cpp heap.cpp heap.h huffman.cpp huffman.h iobitstream.cpp iobitstream.h)
