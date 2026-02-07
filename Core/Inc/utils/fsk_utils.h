#ifndef FSK_UTILS_H
#define FSK_UTILS_H

#include <stddef.h>

typedef struct
{
    size_t sample_rate;      // Recommended sample rate in Hz
    size_t samples_per_bit; // Number of samples per symbol (bit)
} fsk_timing_t;

fsk_timing_t fsk_calculate_timing(float f0, float f1, int baud_rate);

#endif // FSK_UTILS_H
