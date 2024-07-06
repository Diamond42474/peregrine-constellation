# Directory containing internal headers
set(INTERNAL_INCLUDE_DIR 
    ${CMAKE_CURRENT_LIST_DIR}/../peregrine-constellation
    ${CMAKE_CURRENT_LIST_DIR}/../peregrine-constellation/include
)

# Build test suites
add_executable(test_packet_initialization test_packet_initialization.c)
target_include_directories(test_packet_initialization PRIVATE ${INTERNAL_INCLUDE_DIR})
target_link_libraries(test_packet_initialization ${PROJECT_NAME})

add_executable(test_packet_attributes test_packet_attributes.c)
target_include_directories(test_packet_attributes PRIVATE ${INTERNAL_INCLUDE_DIR})
target_link_libraries(test_packet_attributes ${PROJECT_NAME})

add_executable(test_packet_checksum test_packet_checksum.c)
target_include_directories(test_packet_checksum PRIVATE ${INTERNAL_INCLUDE_DIR})
target_link_libraries(test_packet_checksum ${PROJECT_NAME})

# Add test suites
add_test(NAME TestPacketInitialization COMMAND test_packet_initialization)
add_test(NAME TestPacketAttributes COMMAND test_packet_attributes)
add_test(NAME TestPacketChecksum COMMAND test_packet_checksum)
