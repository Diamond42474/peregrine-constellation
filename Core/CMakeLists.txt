# Create the actual implementation library
add_library(peregrine-constellation-impl STATIC
    ${CMAKE_CURRENT_LIST_DIR}/Src/peregrine-constellation.c
    ${CMAKE_CURRENT_LIST_DIR}/Src/packet.c

    ${CMAKE_CURRENT_LIST_DIR}/Src/decoding/decoder.c
    ${CMAKE_CURRENT_LIST_DIR}/Src/decoding/fsk_decoder.c
    ${CMAKE_CURRENT_LIST_DIR}/Src/decoding/byte_assembler.c
    #${CMAKE_CURRENT_LIST_DIR}/Src/decoding/cobs_decoder.c
    ${CMAKE_CURRENT_LIST_DIR}/Src/decoding/packet_decoder.c
    
    ${CMAKE_CURRENT_LIST_DIR}/Src/encoding/encoder.c
    ${CMAKE_CURRENT_LIST_DIR}/Src/encoding/cobs_encoder.c

    ${CMAKE_CURRENT_LIST_DIR}/Src/utils/goertzel.c
    ${CMAKE_CURRENT_LIST_DIR}/Src/utils/circular_buffer.c
    ${CMAKE_CURRENT_LIST_DIR}/Src/utils/fsk_utils.c
)

# Internal include dirs, not exposed to consumers
target_include_directories(peregrine-constellation-impl
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/Inc
)

target_link_libraries(peregrine-constellation-impl
    # Utility libraries
    c-logger     # Logging library
    m           # Math library
)

# Create the interface target
add_library(peregrine-constellation INTERFACE)

target_include_directories(peregrine-constellation
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/Inc
)

# Link the internal implementation to the public interface
target_link_libraries(peregrine-constellation
    INTERFACE
        peregrine-constellation-impl
)
