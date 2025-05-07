# Helper functions for modular builds
macro(add_module target_name)
    add_library(${target_name} SHARED ${ARGN})
    target_include_directories(${target_name}
        PUBLIC
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/external
    )
    target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
endmacro()
