cmake_minimum_required(VERSION 3.5)

project(read_write_file LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(write_file write_file.cpp)
add_executable(read_file read_file.cpp)
