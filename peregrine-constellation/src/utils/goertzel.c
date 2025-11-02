#include "goertzel.h"
#include <math.h>   // For cosf, M_PI
#include <stddef.h> // For NULL
#include "c-logger.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int goertzel_compute_power(const uint16_t *samples, int num_samples, float target_freq, float sample_rate, float *power)
{
    if (!samples || num_samples <= 0 || !power || sample_rate <= 0.0f || target_freq <= 0.0f)
    {
        LOG_ERROR("Invalid parameters: samples=%p, num_samples=%d, power=%p, sample_rate=%.2f, target_freq=%.2f",
                  (void *)samples, num_samples, (void *)power, sample_rate, target_freq);
        return -1; // Invalid parameters
    }

    float s_prev = 0.0f;
    float s_prev2 = 0.0f;

    float normalized_freq = target_freq / sample_rate;
    float coeff = 2.0f * cosf(2.0f * (float)M_PI * normalized_freq);

    for (int i = 0; i < num_samples; i++)
    {
        float s = (float)samples[i] + coeff * s_prev - s_prev2;
        s_prev2 = s_prev;
        s_prev = s;
    }

    *power = s_prev2 * s_prev2 + s_prev * s_prev - coeff * s_prev * s_prev2;

    return 0;
}

int goertzel_compute_power_circular_buff(const circular_buffer_t *cb, int num_samples, float target_freq, float sample_rate, float *power)
{
    if (!cb || num_samples <= 0 || !power || sample_rate <= 0.0f || target_freq <= 0.0f)
    {
        LOG_ERROR("Invalid parameters: cb=%p, num_samples=%d, power=%p, sample_rate=%.2f, target_freq=%.2f",
                  (void *)cb, num_samples, (void *)power, sample_rate, target_freq);
        return -1; // Invalid parameters
    }

    // Copy buffer state to avoid modifying the original buffer
    // Note: do not deallocate the copied buffer
    circular_buffer_t cb_copy = *cb;

    if (circular_buffer_size(&cb_copy) < (size_t)num_samples)
    {
        LOG_ERROR("Not enough samples in circular buffer: required=%d, available=%zu", num_samples, circular_buffer_size(&cb_copy));
        return -1; // Not enough samples
    }

    float s_prev = 0.0f;
    float s_prev2 = 0.0f;

    float normalized_freq = target_freq / sample_rate;
    float coeff = 2.0f * cosf(2.0f * (float)M_PI * normalized_freq);

    for (int i = 0; i < num_samples; i++)
    {
        uint16_t sample;
        if (circular_buffer_pop(&cb_copy, &sample) != 0)
        {
            LOG_ERROR("Failed to peek sample from circular buffer at index %d", i);
            return -1; // Failed to peek sample
        }

        float s = (float)sample + coeff * s_prev - s_prev2;
        s_prev2 = s_prev;
        s_prev = s;
    }

    *power = s_prev2 * s_prev2 + s_prev * s_prev - coeff * s_prev * s_prev2;

    return 0;
}