cmake_minimum_required(VERSION 3.16)

add_library(unity STATIC
    Unity/src/unity.c
)

target_include_directories(unity
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/Unity/src
)

# Optional: silence warnings from third-party code
target_compile_options(unity
    PRIVATE
        -w
)
