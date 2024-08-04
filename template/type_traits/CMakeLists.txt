file(GLOB_RECURSE SRC_FILES "${PROJECT_SOURCE_DIR}/type_traits/*.cpp")

add_executable(type_traits ${SRC_FILES})

list(APPEND flags "-fPIC" "-Wall" "-std=c++2a")

target_compile_options(
    type_traits
    PRIVATE
    ${flags}
)