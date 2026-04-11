cmake_minimum_required(VERSION 3.16)

# Test target name
set(TEST_NAME test_byte_assembler)

set(TEST_SOURCES
    test_byte_assembler.c
)

set(MOCK_SOURCES
    ${PROJECT_SOURCE_DIR}/tests/mocks/mock_decoder.c
)

set(UNIT_SOURCES
    ${PROJECT_SOURCE_DIR}/Core/Src/decoding/byte_assembler.c
    
    ${PROJECT_SOURCE_DIR}/Core/Src/utils/circular_buffer.c
)

set(UNIT_LIBS
    c-logger
)

add_executable(${TEST_NAME}
    ${TEST_SOURCES}
    ${MOCK_SOURCES}
    ${UNIT_SOURCES}
)

# Link unit libraries
target_link_libraries(${TEST_NAME}
    PRIVATE
        ${UNIT_LIBS}
)

# Include paths
target_include_directories(${TEST_NAME}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/Core/Inc
)

# Unity (assumes Unity is already added somewhere higher-level)
# Example: add_subdirectory(external/Unity)
target_link_libraries(${TEST_NAME}
    PRIVATE
        unity
)

# Optional but recommended compile flags
target_compile_options(${TEST_NAME}
    PRIVATE
        # -Wall
        # -Wextra
)

# Register with CTest
add_test(
    NAME ${TEST_NAME}
    COMMAND ${TEST_NAME}
)
