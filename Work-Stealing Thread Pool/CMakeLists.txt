cmake_minimum_required(VERSION 4.3)
project(Work_Stealing_Thread_Pool)

set(CMAKE_CXX_STANDARD 23)

add_executable(Work_Stealing_Thread_Pool main.cpp
        Workpool.hpp)
