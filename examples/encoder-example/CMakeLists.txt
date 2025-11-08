add_executable(encoder_example 
    encoder-example.c
)

target_link_libraries(encoder_example 
    peregrine-constellation
)