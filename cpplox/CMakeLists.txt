cmake_minimum_required(VERSION 3.1...3.19)

project(cpplox VERSION 1.0 LANGUAGES CXX)

add_executable(cpplox
    src/main.cpp
    src/chunk.cpp
)

target_include_directories(cpplox PUBLIC include)

target_compile_options(cpplox PUBLIC
    -W
    -Wall
    -Wextra
)