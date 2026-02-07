#ifndef ENCODER_H
#define ENCODER_H

#include <stddef.h>
#include "utils/circular_buffer.h"
#include "utils/bit_unpacker.h"

typedef enum
{
    ENCODER_TYPE_NONE,
    ENCODER_TYPE_COBS,
} encoder_type_e;

typedef struct
{
    encoder_type_e type;
    circular_buffer_t input_cb;
    size_t input_cb_size;
    circular_buffer_t output_cb;
    size_t output_cb_size;
    bit_unpacker_t bit_unpacker;

    unsigned char preamble_byte; // Normally 0xAA
    size_t preamble_length;      // Number of preamble bytes before each frame
    size_t max_frame_size;       // Default of 256 with COBS

    enum
    {
        ENCODER_UNINITIALIZED,          //< Encoder hasn't been initialized
        ENCODER_IDLE,                   //< Encoder is waiting for data
        ENCODER_WAITING_FOR_FULL_FRAME, //< Encoder is waiting for a full frame (256 using COBS)
        ENCODER_PROCESSING,             //< Encoder is processing frame
    } state;
} encoder_handle_t;

int encoder_init(encoder_handle_t *handle);
int encoder_deinit(encoder_handle_t *handle);

int encoder_set_type(encoder_handle_t *handle, encoder_type_e type);
int encoder_set_input_size(encoder_handle_t *handle, size_t buffer_size);
int encoder_set_output_size(encoder_handle_t *handle, size_t buffer_size);

int encoder_write(encoder_handle_t *handle, const unsigned char *data, size_t len);
int encoder_flush(encoder_handle_t *handle);
int encoder_read(encoder_handle_t *handle, bool *bit);

int encoder_task(encoder_handle_t *handle);
bool encoder_busy(encoder_handle_t *handle);
bool encoder_data_available(encoder_handle_t *handle);

#endif // ENCODER_H