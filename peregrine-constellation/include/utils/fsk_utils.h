#ifndef FSK_UTILS_H
#define FSK_UTILS_H

#include <stddef.h>

typedef struct
{
    float sample_rate;      // Recommended sample rate in Hz
    size_t samples_per_bit; // Number of samples per symbol (bit)
} fsk_timing_t;

/**
 * @brief Calculate optimal sample rate and samples/bit for Goertzel-based FSK decoding.
 *
 * This chooses a sample rate such that both mark and space frequencies
 * fall close to integer Goertzel bins for the given baud rate.
 *
 * @param mark_freq Frequency of logical 1 (Hz)
 * @param space_freq Frequency of logical 0 (Hz)
 * @param baud_rate Symbol rate (bits per second)
 * @param min_fs Minimum ADC sample rate allowed (Hz)
 * @param max_fs Maximum ADC sample rate allowed (Hz)
 * @param min_samples Minimum Goertzel window size (typ 64 or 128)
 * @param max_samples Maximum Goertzel window size (typ 512 or 1024)
 * @return fsk_timing_t Timing parameters (sample rate, samples per bit)
 */
fsk_timing_t fsk_calculate_timing(float mark_freq,
                                  float space_freq,
                                  float baud_rate,
                                  float min_fs,
                                  float max_fs,
                                  size_t min_samples,
                                  size_t max_samples);

#endif // FSK_UTILS_H
