add_executable(basic_example 
    basic-example.c

    # BSP
    ./bsp/adc_bsp.c
    ./bsp/dac_bsp.c
    ./bsp/ptt_bsp.c
    ./bsp/time_bsp.c
)

target_link_libraries(basic_example 
    peregrine-constellation
)