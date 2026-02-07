#ifndef FSK_DECODER_H
#define FSK_DECODER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "utils/circular_buffer.h"
#include "decoding/decoder.h"

typedef struct fsk_decoder_handle
{
    struct
    {
        int sample_size;       // Size of the ADC sample buffer
        int sample_rate;       // Sample rate of the ADC
        float power_threshold; // Power threshold for detecting bits
        float freq_0;          // Frequency representing bit 0
        float freq_1;          // Frequency representing bit 1
    } configs;

    enum
    {
        FSK_DECODER_STATE_UNINITIALIZED,
        FSK_DECODER_STATE_INITIALIZING,
        FSK_DECODER_STATE_IDLE,
        FSK_DECODER_STATE_DECODING,
    } state;
} fsk_decoder_handle_t;

int fsk_decoder_init(fsk_decoder_handle_t *handle);
int fsk_decoder_deinit(fsk_decoder_handle_t *handle);

int fsk_decoder_set_bit_buffer_size(fsk_decoder_handle_t *handle, size_t bit_buffer_size);
int fsk_decoder_set_sample_buffer_multiplier(fsk_decoder_handle_t *handle, size_t multiplier);
int fsk_decoder_set_baud_rate(fsk_decoder_handle_t *handle, int _baud_rate);
int fsk_decoder_set_rates(fsk_decoder_handle_t *handle, int _baud_rate, int _sample_rate);
int fsk_decoder_set_frequencies(fsk_decoder_handle_t *handle, float freq_0, float freq_1);
int fsk_decoder_set_power_threshold(fsk_decoder_handle_t *handle, float _threshold);

int fsk_decoder_task(fsk_decoder_handle_t *handle, decoder_handle_t *ctx);

#endif // FSK_DECODER_H