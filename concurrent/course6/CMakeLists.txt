add_executable(course6 course6.cpp)

list(APPEND flags "-fPIC" "-Wall" "-pthread")

target_compile_options(
    course6
    PRIVATE
    ${flags}
)