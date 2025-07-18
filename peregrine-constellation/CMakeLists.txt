# Create the actual implementation library
add_library(peregrine-constellation-impl STATIC
    ${CMAKE_CURRENT_LIST_DIR}/src/peregrine-constellation.c
    ${CMAKE_CURRENT_LIST_DIR}/src/packet.c
    ${CMAKE_CURRENT_LIST_DIR}/src/decoding/fsk_decoder.c
    ${CMAKE_CURRENT_LIST_DIR}/src/utils/goertzel.c
    ${CMAKE_CURRENT_LIST_DIR}/src/decoding/byte_assembler.c
    ${CMAKE_CURRENT_LIST_DIR}/src/decoding/cobs_decoder.c
    ${CMAKE_CURRENT_LIST_DIR}/src/encoding/cobs_encoder.c
)

# Internal include dirs, not exposed to consumers
target_include_directories(peregrine-constellation-impl
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}/include/decoding
        ${CMAKE_CURRENT_LIST_DIR}/include/encoding
        ${CMAKE_CURRENT_LIST_DIR}/include/utils
)

target_link_libraries(peregrine-constellation-impl
    # Utility libraries
    c-logger
)

# Create the interface target
add_library(peregrine-constellation INTERFACE)

target_include_directories(peregrine-constellation
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/include/decoding
        ${CMAKE_CURRENT_LIST_DIR}/include/encoding

)

# Link the internal implementation to the public interface
target_link_libraries(peregrine-constellation
    INTERFACE
        peregrine-constellation-impl
)
