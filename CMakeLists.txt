cmake_minimum_required(VERSION 3.13)

set(PROJECT_NAME peregrine-constellation)
project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES C CXX)

# Specify C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

# Include directories
include_directories(include)

# Source files
set(SOURCES
    src/peregrine-constellation.c
    src/packet.c
)

# Create static library
add_library(${PROJECT_NAME} STATIC ${SOURCES})

# Add examples
add_subdirectory(examples)

# Add tests
enable_testing()
add_subdirectory(tests)
