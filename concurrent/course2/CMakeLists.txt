add_executable(course2 course2.cpp)

list(APPEND flags "-fPIC" "-Wall" "-pthread")

target_compile_options(
    course2
    PRIVATE
    ${flags}
)