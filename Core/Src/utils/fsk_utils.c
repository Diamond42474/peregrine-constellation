#include "utils/fsk_utils.h"
#include <math.h>

fsk_timing_t fsk_calculate_timing(float f0, float f1, int baud_rate)
{
    fsk_timing_t result;
    float max_frequency = 0;
    if (f0 > f1)
    {
        max_frequency = f0;
    }
    else
    {
        max_frequency = f1;
    }

    result.sample_rate = (size_t)(max_frequency * 3);
    result.samples_per_bit = (size_t)(result.sample_rate / baud_rate);

    return result;
}