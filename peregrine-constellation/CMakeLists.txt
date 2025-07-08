# Create the actual implementation library
add_library(peregrine-constellation-impl STATIC
    ${CMAKE_CURRENT_LIST_DIR}/src/peregrine-constellation.c
    ${CMAKE_CURRENT_LIST_DIR}/src/packet.c
    ${CMAKE_CURRENT_LIST_DIR}/src/utils/fsk_decoder.c
    ${CMAKE_CURRENT_LIST_DIR}/src/utils/goertzel.c
)

# Internal include dirs, not exposed to consumers
target_include_directories(peregrine-constellation-impl
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/include
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
)

# Link the internal implementation to the public interface
target_link_libraries(peregrine-constellation
    INTERFACE
        peregrine-constellation-impl
)
