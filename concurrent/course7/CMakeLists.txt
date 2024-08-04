add_executable(course7 course7.cpp)

list(APPEND flags "-fPIC" "-Wall" "-pthread")

target_compile_options(
    course7
    PRIVATE
    ${flags}
)