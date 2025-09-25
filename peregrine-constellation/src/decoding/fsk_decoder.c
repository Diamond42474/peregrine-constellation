#include "fsk_decoder.h"

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "goertzel.h"
#include "c-logger.h"

#define BUFFER_SIZE_MULTIPLYER 2 // Buffer size multiplier for ADC samples

static int baud_rate = 8;                  // Default baud rate
static int sample_rate = 4400;             // 2400Hz Default sample rate
static float freq_0 = 1100.0f;             // Default frequency for bit 0
static float freq_1 = 2200.0f;             // Default frequency for bit 1
static float power_threshold = 1000000.0f; // Default power threshold
static bool initialized = false;
static bool running = false;
static bool adc_samples_ready = false;

static uint16_t *adc_samples = NULL;   // Buffer for ADC samples, size can be adjusted
static int adc_sample_buffer_size = 0; // Buffer size for ADC samples
static int adc_sample_size = 0;        // Size of the ADC samples

static void adc_sample_callback(size_t size);
static void _process_samples(fsk_decoder_handle_t *handle, const uint16_t *samples, size_t num_samples);
static int _calculate_window_offset(void);

int fsk_decoder_init(fsk_decoder_handle_t *handle)
{
    int ret = 0;

    if (initialized)
    {
        LOG_WARN("FSK decoder already initialized");
        return 0; // Already initialized
    }

    if (goertzel_init())
    {
        LOG_ERROR("Failed to initialize Goertzel algorithm");
        ret = -1;
        goto failed;
    }

    if (handle->adc_init())
    {
        LOG_ERROR("Failed to initialize ADC");
        ret = -1;
        goto failed;
    }

    if (handle->adc_set_callback(adc_sample_callback))
    {
        LOG_ERROR("Failed to set ADC sample callback");
        ret = -1;
        goto failed;
    }

    initialized = true;

failed:
    return ret;
}

int fsk_decoder_deinit(fsk_decoder_handle_t *handle)
{
    int ret = 0;

failed:
    return ret;
}

int fsk_decoder_set_baud_rate(fsk_decoder_handle_t *handle, int _baud_rate)
{
    int ret = 0;

    if (_baud_rate <= 0)
    {
        LOG_ERROR("Invalid baud rate: %d", _baud_rate);
        ret = -1;
        goto failed;
    }

    baud_rate = _baud_rate;

failed:
    return ret;
}

int fsk_decoder_set_sample_rate(fsk_decoder_handle_t *handle, int _sample_rate)
{
    int ret = 0;

    if (sample_rate <= 0)
    {
        LOG_ERROR("Invalid sample rate: %d", sample_rate);
        ret = -1;
        goto failed;
    }

    sample_rate = _sample_rate;

failed:
    return ret;
}

int fsk_decoder_set_frequencies(fsk_decoder_handle_t *handle, float _freq_0, float _freq_1)
{
    int ret = 0;

    if (!initialized)
    {
        if (fsk_decoder_init(handle))
        {
            LOG_ERROR("Failed to initialize FSK decoder");
            ret = -1;
            goto failed;
        }
    }

    if (_freq_0 <= 0 || _freq_1 <= 0)
    {
        LOG_ERROR("Invalid frequencies: freq_0 = %f, freq_1 = %f", _freq_0, _freq_1);
        ret = -1;
        goto failed;
    }

    if (_freq_0 == _freq_1)
    {
        LOG_ERROR("Frequencies must be different: freq_0 = %f, freq_1 = %f", _freq_0, _freq_1);
        ret = -1;
        goto failed;
    }

    freq_0 = _freq_0;
    freq_1 = _freq_1;

failed:
    return ret;
}

int fsk_decoder_set_power_threshold(fsk_decoder_handle_t *handle, float _threshold)
{
    int ret = 0;

    if (!initialized)
    {
        if (fsk_decoder_init(handle))
        {
            LOG_ERROR("Failed to initialize FSK decoder");
            ret = -1;
            goto failed;
        }
    }

    if (_threshold <= 0)
    {
        LOG_ERROR("Invalid power threshold: %f", _threshold);
        ret = -1;
        goto failed;
    }

    power_threshold = _threshold;

failed:
    return ret;
}

int fsk_decoder_start(fsk_decoder_handle_t *handle)
{
    int ret = 0;

    if (!initialized)
    {
        if (fsk_decoder_init(handle))
        {
            LOG_ERROR("Failed to initialize FSK decoder");
            ret = -1;
            goto failed;
        }
    }

    if (running)
    {
        LOG_WARN("FSK decoder already running");
        return 0; // Already running
    }

    if (handle->adc_set_sample_rate(sample_rate))
    {
        LOG_ERROR("Failed to set ADC sample rate: %d", sample_rate);
        ret = -1;
        goto failed;
    }

    adc_sample_size = sample_rate / baud_rate;

    if (handle->adc_set_sample_size(adc_sample_size))
    {
        LOG_ERROR("Failed to set ADC sample size: %d", adc_sample_size);
        ret = -1;
        goto failed;
    }

    int buffer_size = adc_sample_size * BUFFER_SIZE_MULTIPLYER; // Gives some extra space for bit alignment

    if (adc_sample_buffer_size == 0) // Allocate buffer if not already allocated
    {
        adc_samples = calloc(buffer_size, sizeof(uint16_t));
        if (adc_samples == NULL)
        {
            LOG_ERROR("Failed to allocate memory for ADC sample buffer");
            ret = -1;
            goto failed;
        }
        adc_sample_buffer_size = buffer_size;
    }
    else if (adc_sample_buffer_size < buffer_size) // Resize the buffer if it's smaller than required
    {
        free(adc_samples);
        adc_samples = calloc(buffer_size, sizeof(uint16_t));
        if (adc_samples == NULL)
        {
            LOG_ERROR("Failed to reallocate memory for ADC sample buffer");
            ret = -1;
            goto failed;
        }
        adc_sample_buffer_size = buffer_size;
    }

    // Start the ADC sampling
    if (handle->adc_start())
    {
        LOG_ERROR("Failed to start ADC sampling");
        ret = -1;
        goto failed;
    }

    running = true;
failed:
    return ret;
}

int fsk_decoder_stop(fsk_decoder_handle_t *handle)
{
    int ret = 0;

    if (!initialized)
    {
        if (fsk_decoder_init(handle))
        {
            LOG_ERROR("Failed to initialize FSK decoder");
            ret = -1;
            goto failed;
        }
    }

    if (!running)
    {
        LOG_WARN("FSK decoder not running");
        return 0; // Already stopped
    }

    // Stop the ADC sampling
    if (handle->adc_stop())
    {
        LOG_ERROR("Failed to stop ADC sampling");
        ret = -1;
        goto failed;
    }

    running = false;

failed:
    return ret;
}

int fsk_decoder_is_running(fsk_decoder_handle_t *handle)
{
    if (!initialized)
    {
        LOG_WARN("FSK decoder not initialized");
        return 0; // Not running
    }

    return running ? 1 : 0; // Return 1 if running, 0 if not
}

int fsk_decoder_process(fsk_decoder_handle_t *handle)
{
    int ret = 0;

    if (!initialized)
    {
        if (fsk_decoder_init(handle))
        {
            LOG_ERROR("Failed to initialize FSK decoder");
            ret = -1;
            goto failed;
        }
    }

    if (!running)
    {
        LOG_WARN("FSK decoder not running");
        return 0; // Not running
    }

    if (!adc_samples_ready)
    {
        return 0; // No samples to process
    }

    // Move old samples to the beginning of the buffer
    memcpy(adc_samples, adc_samples + adc_sample_size, (adc_sample_buffer_size - adc_sample_size) * sizeof(uint16_t));

    adc_samples_ready = false; // Reset the flag
    int samples_read = 0;

    // Get new ADC samples
    if (handle->adc_get_samples(adc_samples + (adc_sample_buffer_size - adc_sample_size), adc_sample_size, &samples_read))
    {
        LOG_ERROR("Failed to get ADC samples");
        ret = -1;
        goto failed;
    }
    if (samples_read != adc_sample_size)
    {
        LOG_WARN("Expected %d samples, but got %d", adc_sample_size, samples_read);
        ret = -1;
        goto failed;
    }

    LOG_DEBUG("Processing %d ADC samples", adc_sample_size);
    int window_offset = 0; //_calculate_window_offset();
    _process_samples(handle, adc_samples + window_offset, adc_sample_size);

failed:
    return ret;
}

int fsk_decoder_set_adc_callbacks(fsk_decoder_handle_t *handle,
                                  int (*adc_init)(void),
                                  int (*adc_start)(void),
                                  int (*adc_stop)(void),
                                  int (*adc_set_callback)(void (*callback)(size_t size)),
                                  int (*adc_set_sample_rate)(int rate),
                                  int (*adc_set_sample_size)(int size),
                                  int (*adc_get_samples)(uint16_t *buffer, size_t size, int *samples_read))
{
    int ret = 0;

    if (!initialized)
    {
        if (fsk_decoder_init(handle))
        {
            LOG_ERROR("Failed to initialize FSK decoder");
            ret = -1;
            goto failed;
        }
    }

    if (adc_init == NULL || adc_start == NULL || adc_stop == NULL || adc_set_callback == NULL ||
        adc_set_sample_rate == NULL || adc_set_sample_size == NULL || adc_get_samples == NULL)
    {
        LOG_ERROR("ADC callback functions cannot be NULL");
        ret = -1;
        goto failed;
    }

    // Set the ADC callbacks
    handle->adc_init = adc_init;
    handle->adc_start = adc_start;
    handle->adc_stop = adc_stop;
    handle->adc_set_callback = adc_set_callback;
    handle->adc_set_sample_rate = adc_set_sample_rate;
    handle->adc_set_sample_size = adc_set_sample_size;
    handle->adc_get_samples = adc_get_samples;

failed:
    return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// PRIVATE FUNCTIONS
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=

static void adc_sample_callback(size_t size)
{
    adc_samples_ready = true;
}

static void _process_samples(fsk_decoder_handle_t *handle, const uint16_t *samples, size_t num_samples)
{
    float power_0 = 0.0f;
    float power_1 = 0.0f;

    if (goertzel_compute_power(samples, num_samples, freq_0, sample_rate, &power_0) < 0)
    {
        LOG_ERROR("Failed to compute power for frequency %f", freq_0);
        return;
    }

    if (goertzel_compute_power(samples, num_samples, freq_1, sample_rate, &power_1) < 0)
    {
        LOG_ERROR("Failed to compute power for frequency %f", freq_1);
        return;
    }

    //LOG_INFO("freq_0: %f, freq_1: %f", power_0, power_1);
    //LOG_INFO("f0: %f, f1: %f", freq_0, freq_1);

    if (power_0 < power_threshold && power_1 < power_threshold)
    {
        LOG_DEBUG("No significant signal detected (power_0: %f, power_1: %f)", power_0, power_1);
        return; // No significant signal detected
    }
    LOG_DEBUG("Significant signal detected (power_0: %f, power_1: %f)", power_0, power_1);

    int bit = -1;

    if (power_0 > power_1)
    {
        bit = 0; // Detected frequency 0
    }
    else
    {
        bit = 1; // Detected frequency 1
    }

    //LOG_INFO("Detected bit: %d", bit);
    if (!handle->bit_callback)
    {
        LOG_ERROR("Bit callback not set");
        return; // No callback to handle detected bits
    }

    handle->bit_callback(bit); // Call the callback with the detected bit
}

static int _calculate_window_offset(void)
{
    int sample_size = adc_sample_size;
    const int max_offset_range = sample_size / 4;

    // Calculate the offset based off of the offset that gives the greatest power difference
    int max_power_diff = 0;
    int best_offset = 0;
    for (int offset = 0; offset < max_offset_range; offset++)
    {
        float power_0 = 0.0f;
        float power_1 = 0.0f;

        if (goertzel_compute_power(adc_samples + offset, sample_size, freq_0, sample_rate, &power_0) < 0)
        {
            LOG_ERROR("Failed to compute power for frequency %f at offset %d", freq_0, offset);
            continue;
        }

        if (goertzel_compute_power(adc_samples + offset, sample_size, freq_1, sample_rate, &power_1) < 0)
        {
            LOG_ERROR("Failed to compute power for frequency %f at offset %d", freq_1, offset);
            continue;
        }

        float power_diff = fabsf(power_1 - power_0);
        if (power_diff > max_power_diff)
        {
            max_power_diff = power_diff;
            best_offset = offset;
        }
    }

    if (max_power_diff <= 0)
    {
        return 0; // No significant power difference found, use default offset
    }

    LOG_DEBUG("Best offset found: %d with power difference: %d", best_offset, max_power_diff);
    return best_offset; // Return the best offset found
}