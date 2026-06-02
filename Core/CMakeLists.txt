# Create the actual implementation library
add_library(peregrine-constellation-impl STATIC
    ${CMAKE_CURRENT_LIST_DIR}/Src/peregrine-constellation.c
    ${CMAKE_CURRENT_LIST_DIR}/Src/packet.c

    ${CMAKE_CURRENT_LIST_DIR}/Src/decoding/decoder.c
    ${CMAKE_CURRENT_LIST_DIR}/Src/decoding/fsk_decoder.c
    ${CMAKE_CURRENT_LIST_DIR}/Src/decoding/byte_assembler.c
    ${CMAKE_CURRENT_LIST_DIR}/Src/decoding/packet_decoder.c

    ${CMAKE_CURRENT_LIST_DIR}/Src/dsp/filters.c
    
    ${CMAKE_CURRENT_LIST_DIR}/Src/encoding/packet_serializer.c

    ${CMAKE_CURRENT_LIST_DIR}/Src/utils/time_utils.c
    ${CMAKE_CURRENT_LIST_DIR}/Src/utils/goertzel.c
    ${CMAKE_CURRENT_LIST_DIR}/Src/utils/circular_buffer.c
    ${CMAKE_CURRENT_LIST_DIR}/Src/utils/fsk_utils.c

    ${CMAKE_CURRENT_LIST_DIR}/Src/orchestrator.c
    ${CMAKE_CURRENT_LIST_DIR}/Src/modem.c
)

# Internal include dirs, not exposed to consumers
target_include_directories(peregrine-constellation-impl
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/Inc
)

target_link_libraries(peregrine-constellation-impl
    # Utility libraries
    c-logger     # Logging library
    m           # Math library
)

# Create the interface target
add_library(peregrine-constellation INTERFACE)

# Public include dirs for consumers of the library
target_include_directories(peregrine-constellation
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/Inc/interface
        ${CMAKE_CURRENT_LIST_DIR}/Inc
)

# Link the internal implementation to the public interface
target_link_libraries(peregrine-constellation
    INTERFACE
        peregrine-constellation-impl
)
