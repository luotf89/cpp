add_executable(course4 course4.cpp)

list(APPEND flags "-fPIC" "-Wall" "-pthread")

target_compile_options(
    course4
    PRIVATE
    ${flags}
)