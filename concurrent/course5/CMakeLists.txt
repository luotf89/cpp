add_executable(course5 course5.cpp)

list(APPEND flags "-fPIC" "-Wall" "-pthread")

target_compile_options(
    course5
    PRIVATE
    ${flags}
)