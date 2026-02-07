#ifndef COBS_ENCODER_H
#define COBS_ENCODER_H

#include <stddef.h>
#include <stdint.h>
#include "utils/circular_buffer.h"

int cobs_encode(const uint8_t *input, size_t input_len, uint8_t *output, size_t output_len);
int cobs_encode_cb(circular_buffer_t *input_cb, size_t input_size, circular_buffer_t *output_cb);

#endif // COBS_ENCODER_H