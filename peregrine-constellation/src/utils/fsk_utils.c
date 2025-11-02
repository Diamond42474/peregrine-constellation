#include "fsk_utils.h"
#include <math.h>

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
                                  size_t max_samples)
{
    fsk_timing_t result = {0};
    float best_error = 1e9f;
    float target_freqs[2] = {mark_freq, space_freq};

    // Search through possible sample rates and window sizes
    for (size_t N = min_samples; N <= max_samples; N *= 2)
    {
        for (float fs = min_fs; fs <= max_fs; fs *= 1.05f)
        {
            float total_error = 0.0f;

            for (int i = 0; i < 2; i++)
            {
                float f = target_freqs[i];
                float k = roundf((f * N) / fs); // nearest bin
                float f_actual = (k * fs) / N;
                total_error += fabsf(f_actual - f);
            }

            if (total_error < best_error)
            {
                best_error = total_error;
                result.sample_rate = fs;
                result.samples_per_bit = (size_t)roundf(fs / baud_rate);
            }
        }
    }

    return result;
}
