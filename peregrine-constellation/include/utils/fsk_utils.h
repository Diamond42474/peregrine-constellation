#ifndef FSK_UTILS_H
#define FSK_UTILS_H

#include <stddef.h>

typedef struct
{
    float sample_rate;      // Recommended sample rate in Hz
    size_t samples_per_bit; // Number of samples per symbol (bit)
} fsk_timing_t;

/**
 * @brief Calculate sample rate and sample size for FSK decoding.
 *
 * @param mark_freq Frequency of logical 1 (Hz)
 * @param space_freq Frequency of logical 0 (Hz)
 * @param baud_rate Symbol rate (bits per second)
 * @param oversample_factor Desired number of samples per cycle (>=2)
 * @return fsk_timing_t Calculated timing parameters
 */
fsk_timing_t fsk_calculate_timing(float mark_freq,
                                  float space_freq,
                                  float baud_rate,
                                  float oversample_factor);

#endif // FSK_UTILS_H
