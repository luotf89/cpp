cmake_minimum_required(VERSION 3.22.0)
project(algorithm VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

add_compile_options(
    -fcoroutines
)

add_subdirectory(./function)
add_subdirectory(./tree/bst)
add_subdirectory(./tree/avl)
add_subdirectory(./tree/rbt)
