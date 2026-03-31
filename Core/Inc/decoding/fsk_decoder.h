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
        size_t symbol_sample_size;  // Size of the ADC sample buffer
        int sample_rate;            // Sample rate of the ADC
        size_t buffer_symbol_count; // Number of symbols to buffer for processing
        float power_threshold;      // Power threshold for detecting bits
        float freq_0;               // Frequency representing bit 0
        float freq_1;               // Frequency representing bit 1
    } configs;

    size_t symbol_timing_offset;
    bool signal_detected;

    enum
    {
        FSK_DECODER_STATE_UNINITIALIZED,
        FSK_DECODER_STATE_INITIALIZING,
        FSK_DECODER_STATE_IDLE,
        FSK_DECODER_WAITING_FOR_SIGNAL, ///< Waiting for signal to exceed quality threshold
        FSK_DECODER_RECOVERING_TIMING,  ///< Attempting to recover symbol timing
        FSK_DECODER_STATE_DECODING,     ///< Actively decoding samples
    } state;
} fsk_decoder_handle_t;

int fsk_decoder_init(fsk_decoder_handle_t *handle);
int fsk_decoder_deinit(fsk_decoder_handle_t *handle);

int fsk_decoder_set_symbol_sample_size(fsk_decoder_handle_t *handle, size_t _symbol_sample_size, size_t _buffer_symbol_count);
int fsk_decoder_set_sample_rate(fsk_decoder_handle_t *handle, int sample_rate);
int fsk_decoder_set_frequencies(fsk_decoder_handle_t *handle, float freq_0, float freq_1);
int fsk_decoder_set_power_threshold(fsk_decoder_handle_t *handle, float _threshold);
int fsk_decoder_reset_symbol_timing(fsk_decoder_handle_t *handle);

int fsk_decoder_task(fsk_decoder_handle_t *handle, decoder_handle_t *ctx);
bool fsk_decoder_busy(fsk_decoder_handle_t *handle, decoder_handle_t *ctx);

#endif // FSK_DECODER_H
