#ifndef COBS_DECODER_H
#define COBS_DECODER_H

#include <stddef.h>
#include <stdint.h>
#include "circular_buffer.h"

typedef struct
{
    circular_buffer_t input_cb;
    size_t input_buffer_size;

    unsigned char frame_buffer[256];
    unsigned char code;
    size_t write_index;
    size_t remaining;

    circular_buffer_t output_cb;
    size_t output_buffer_size;

    enum {
        COBS_DECODER_STATE_UNINITIALIZED,
        COBS_DECODER_STATE_INITIALIZING,
        COBS_DECODER_STATE_IDLE,
        COBS_DECODER_STATE_DECODING,
    } state;
} cobs_decoder_t;

int cobs_decoder_init(cobs_decoder_t *handle);
int cobs_decoder_deinit(cobs_decoder_t *handle);
int cobs_decoder_set_input_buffer_size(cobs_decoder_t *handle, size_t size);
int cobs_decoder_set_output_buffer_size(cobs_decoder_t *handle, size_t size);

int cobs_decoder_process(cobs_decoder_t *handle, unsigned char byte);

bool cobs_decoder_has_message(cobs_decoder_t *handle);
int cobs_decoder_get_message(cobs_decoder_t *handle, unsigned char *buffer, size_t buffer_len, size_t *message_len);

void cobs_decoder_task(cobs_decoder_t *handle);
bool cobs_decoder_busy(cobs_decoder_t *handle);

#endif // COBS_DECODER_H