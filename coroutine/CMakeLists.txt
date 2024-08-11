cmake_minimum_required(VERSION 3.22.0)
project(coroutine VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(
    -fcoroutines
)

include_directories(/usr/include/c++/11)

add_subdirectory(./00.call)
add_subdirectory(./01.generator)
add_subdirectory(./02.task)

