#ifndef COBS_ENCODER_H
#define COBS_ENCODER_H

#include <stddef.h>
#include <stdint.h>

int cobs_encode(const uint8_t *input, size_t input_len, uint8_t *output, size_t output_len);

#endif // COBS_ENCODER_H