# Build test suites
add_executable(test_packet_initialization test_packet_initialization.c)
target_link_libraries(test_packet_initialization ${PROJECT_NAME})

add_executable(test_packet_attributes test_packet_attributes.c)
target_link_libraries(test_packet_attributes ${PROJECT_NAME})

add_executable(test_packet_checksum test_packet_checksum.c)
target_link_libraries(test_packet_checksum ${PROJECT_NAME})

# Add test suites
add_test(NAME TestPacketInitialization COMMAND test_packet_initialization)
add_test(NAME TestPacketAttributes COMMAND test_packet_attributes)
add_test(NAME TestPacketChecksum COMMAND test_packet_checksum)
