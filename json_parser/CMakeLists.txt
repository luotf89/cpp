cmake_minimum_required(VERSION 3.27)
project(json_parser_demo LANGUAGES CXX)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

list(APPEND flags "-fPIC" "-Wall")
add_library(json_parser STATIC 
  json.h json.cpp
  lexer.h lexer.cpp
  parser.h parser.cpp)

target_compile_options(json_parser PRIVATE ${flags})

add_executable(json_parser_demo main.cpp)

target_link_libraries(json_parser_demo json_parser)

