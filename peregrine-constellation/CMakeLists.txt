add_library(peregrine-constellation INTERFACE)

target_sources(peregrine-constellation
    INTERFACE 
      ${CMAKE_CURRENT_LIST_DIR}/src/peregrine-constellation.c
      ${CMAKE_CURRENT_LIST_DIR}/src/packet.c
)

target_include_directories(peregrine-constellation
    INTERFACE 
      ${CMAKE_CURRENT_LIST_DIR}/include
)