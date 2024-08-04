cmake_minimum_required(VERSION 3.27.2)
project(concurrent VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

include(CTest)
enable_testing()

add_subdirectory(./course1)
add_subdirectory(./course2)
add_subdirectory(./course3)
add_subdirectory(./course4)
add_subdirectory(./course5)
add_subdirectory(./course6)
add_subdirectory(./course7)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
