cmake_minimum_required(VERSION 3.22.0)
project(coroutine VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(
    -fcoroutines
)

# include_directories(/usr/include/c++/11)
include_directories(./utils)

add_subdirectory(./00.previous_awaiter)
add_subdirectory(./01.generator)
add_subdirectory(./02.task)
add_subdirectory(./03.timer)
add_subdirectory(./04.when_any)

