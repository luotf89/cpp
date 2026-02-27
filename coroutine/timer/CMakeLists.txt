add_executable(timer timer.cpp)

list(APPEND flags "-fPIC" "-Wall" "-pthread")

target_compile_options(
    timer
    PRIVATE
    ${flags}
)

# target_link_libraries(generator private
#     lib
# )