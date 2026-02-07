#ifndef COBS_DECODER_H
#define COBS_DECODER_H

#include <stddef.h>
#include <stdint.h>
#include "decoder.h"

typedef struct
{
    unsigned char frame_buffer[256];
    unsigned char code;
    size_t write_index;
    size_t remaining;
} cobs_decoder_t;

int cobs_decoder_init(cobs_decoder_t *handle);
int cobs_decoder_deinit(cobs_decoder_t *handle);

int cobs_decoder_process(cobs_decoder_t *handle, decoder_handle_t *ctx, unsigned char byte);

#endif // COBS_DECODER_H