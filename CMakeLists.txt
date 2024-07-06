cmake_minimum_required(VERSION 3.13)

set(PROJECT_NAME peregrine-constellation)
project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES C CXX)

# Specify C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

# Add main project
add_subdirectory(peregrine-constellation)

# Add examples
add_subdirectory(examples)

# Add tests
enable_testing()
add_subdirectory(tests)
