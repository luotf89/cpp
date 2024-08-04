add_executable(course3 course3.cpp)

list(APPEND flags "-fPIC" "-Wall" "-pthread")

target_compile_options(
    course3
    PRIVATE
    ${flags}
)