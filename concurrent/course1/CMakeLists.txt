add_executable(course1 course1.cpp)

list(APPEND flags "-fPIC" "-Wall" "-pthread")

target_compile_options(
    course1
    PRIVATE
    ${flags}
)