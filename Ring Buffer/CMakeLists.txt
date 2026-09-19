cmake_minimum_required(VERSION 4.3)
project(Ring_Buffer)

set(CMAKE_CXX_STANDARD 20)

add_executable(Ring_Buffer main.cpp
        include/concepts.hpp
        include/buffer.hpp)
