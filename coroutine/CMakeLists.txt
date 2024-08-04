cmake_minimum_required(VERSION 3.22.0)
project(coroutine VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

add_compile_options(
    -fcoroutines
)

add_subdirectory(./generator)

