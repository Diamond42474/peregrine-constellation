#include "fsk_utils.h"
#include <math.h>

fsk_timing_t fsk_calculate_timing(float mark_freq,
                                  float space_freq,
                                  float baud_rate,
                                  float oversample_factor)
{
    fsk_timing_t result = {0};

    // Determine the highest tone
    float f_max = (mark_freq > space_freq) ? mark_freq : space_freq;

    // Clamp oversampling to minimum Nyquist requirement
    if (oversample_factor < 2.0f)
        oversample_factor = 2.0f;

    // Compute sample rate
    result.sample_rate = f_max * oversample_factor;

    // Compute samples per bit
    result.samples_per_bit = (size_t)(result.sample_rate / baud_rate);

    // Optional: adjust sample rate to be an integer multiple of baud_rate
    // This makes timing align nicely
    result.sample_rate = baud_rate * (float)result.samples_per_bit;

    return result;
}
