#ifndef COBS_DECODER_H
#define COBS_DECODER_H

#include <stddef.h>
#include <stdint.h>

// Callback when a full decoded frame is ready
int cobs_decoder_set_data_callback(void (*data_callback)(const uint8_t *data, size_t len));

int cobs_decoder_init(void);
int cobs_decoder_deinit(void);

int cobs_decoder_reset(void);
void cobs_decoder_input(uint8_t byte);

#endif // COBS_DECODER_H