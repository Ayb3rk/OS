cmake_minimum_required(VERSION 3.21)
project(bundle C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(bundle
        main.c
        parser.c
        parser.h)
