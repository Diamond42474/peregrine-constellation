add_executable(decoder_example 
    decoder-example.c
)

target_link_libraries(decoder_example 
    peregrine-constellation
)