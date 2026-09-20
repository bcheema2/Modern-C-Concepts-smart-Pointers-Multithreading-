cmake_minimum_required(VERSION 4.3)
project(Ring_Buffer)

set(CMAKE_CXX_STANDARD 20)

find_package(Threads REQUIRED)

include_directories(include)

add_executable(Ring_Buffer
        main.cpp
)
target_link_libraries(Ring_Buffer PRIVATE Threads::Threads)

add_executable(RingBufferBenchmark
        benchmark.cpp
)
target_link_libraries(RingBufferBenchmark PRIVATE Threads::Threads)
