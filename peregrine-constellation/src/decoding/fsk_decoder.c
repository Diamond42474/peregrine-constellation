#include "fsk_decoder.h"

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "goertzel.h"
#include "c-logger.h"
#include "circular_buffer.h"

static int _process_samples(fsk_decoder_handle_t *handle, decoder_handle_t *ctx);
static int _calculate_window_offset(fsk_decoder_handle_t *handle, uint16_t *samples, size_t num_samples);

int fsk_decoder_init(fsk_decoder_handle_t *handle)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("FSK decoder handle is NULL");
        return -1;
    }

    handle->state = FSK_DECODER_STATE_INITIALIZING;

failed:
    return ret;
}

int fsk_decoder_deinit(fsk_decoder_handle_t *handle)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("FSK decoder handle is NULL");
        return -1;
    }

    if (handle->state == FSK_DECODER_STATE_UNINITIALIZED)
    {
        LOG_WARN("FSK decoder is already uninitialized");
        return 0;
    }

    handle->state = FSK_DECODER_STATE_UNINITIALIZED;

failed:
    return ret;
}

int fsk_decoder_set_bit_buffer_size(fsk_decoder_handle_t *handle, size_t bit_buffer_size)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("FSK decoder handle is NULL");
        ret = -1;
        goto failed;
    }

    // handle->configs.bit_buffer_size = bit_buffer_size;

failed:
    return ret;
}

int fsk_decoder_set_rates(fsk_decoder_handle_t *handle, int _sample_size, int _sample_rate)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("FSK decoder handle is NULL");
        ret = -1;
        goto failed;
    }

    if (_sample_size <= 0)
    {
        LOG_ERROR("Invalid sample size: %d", _sample_size);
        ret = -1;
        goto failed;
    }

    if (_sample_rate <= 0)
    {
        LOG_ERROR("Invalid sample rate: %d", handle->configs.sample_rate);
        ret = -1;
        goto failed;
    }

    handle->configs.sample_rate = _sample_rate;
    handle->configs.sample_size = _sample_size;

failed:
    return ret;
}

int fsk_decoder_set_frequencies(fsk_decoder_handle_t *handle, float _freq_0, float _freq_1)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("FSK decoder handle is NULL");
        ret = -1;
        goto failed;
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

    handle->configs.freq_0 = _freq_0;
    handle->configs.freq_1 = _freq_1;

failed:
    return ret;
}

int fsk_decoder_set_power_threshold(fsk_decoder_handle_t *handle, float _threshold)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("FSK decoder handle is NULL");
        ret = -1;
        goto failed;
    }

    if (_threshold <= 0)
    {
        LOG_ERROR("Invalid power threshold: %f", _threshold);
        ret = -1;
        goto failed;
    }

    handle->configs.power_threshold = _threshold;

failed:
    return ret;
}

int fsk_decoder_task(fsk_decoder_handle_t *handle, decoder_handle_t *ctx)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("FSK decoder handle is NULL");
        return -1;
    }

    switch (handle->state)
    {
    case FSK_DECODER_STATE_UNINITIALIZED:
        LOG_ERROR("FSK decoder is uninitialized");
        ret = -1;
        break;
    case FSK_DECODER_STATE_INITIALIZING:
        handle->state = FSK_DECODER_STATE_IDLE;
        LOG_INFO("FSK decoder initialized");
        break;
    case FSK_DECODER_STATE_IDLE:
        if (!circular_buffer_is_empty(&ctx->input_buffer))
        {
            handle->state = FSK_DECODER_STATE_DECODING;
        }

        break;
    case FSK_DECODER_STATE_DECODING:
        LOG_DEBUG("Decoding samples, current sample buffer size: %zu", circular_buffer_count(&ctx->input_buffer));
        if (circular_buffer_count(&ctx->input_buffer) >= handle->configs.sample_size)
        {
            if (_process_samples(handle, ctx))
            {
                LOG_ERROR("Failed to process samples");
                ret = -1;
                goto failed;
            }
        }
        break;
    default:
        LOG_ERROR("Unknown FSK decoder state");
        ret = -1;
        break;
    }
failed:
    return ret;
}

bool fsk_decoder_busy(fsk_decoder_handle_t *handle)
{
    if (!handle)
    {
        LOG_ERROR("FSK decoder handle is NULL");
        return false;
    }

    return handle->state != FSK_DECODER_STATE_IDLE;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// PRIVATE FUNCTIONS
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=

static int _process_samples(fsk_decoder_handle_t *handle, decoder_handle_t *ctx)
{
    int ret = 0;

    float power_0 = 0.0f;
    float power_1 = 0.0f;

    if (goertzel_compute_power_circular_buff(&ctx->input_buffer, handle->configs.sample_size, handle->configs.freq_0, handle->configs.sample_rate, &power_0) < 0)
    {
        LOG_ERROR("Failed to compute power for frequency %f", handle->configs.freq_0);
        ret = -1;
        goto failed;
    }
    if (goertzel_compute_power_circular_buff(&ctx->input_buffer, handle->configs.sample_size, handle->configs.freq_1, handle->configs.sample_rate, &power_1) < 0)
    {
        LOG_ERROR("Failed to compute power for frequency %f", handle->configs.freq_1);
        ret = -1;
        goto failed;
    }

    if (power_0 < handle->configs.power_threshold && power_1 < handle->configs.power_threshold)
    {
        LOG_DEBUG("No significant signal detected (power_0: %f, power_1: %f)", power_0, power_1);
        ret - 2;
        goto cleanup;
    }
    LOG_DEBUG("Significant signal detected (power_0: %f, power_1: %f)", power_0, power_1);

    bool bit;

    if (power_0 > power_1)
    {
        bit = false; // Detected frequency 0
    }
    else
    {
        bit = true; // Detected frequency 1
    }

    LOG_DEBUG("Bit: %d", bit);

    if (decoder_process_bit(ctx, bit))
    {
        LOG_ERROR("Failed to process decoded bit");
        ret = -1;
        goto failed;
    }

    // Remove processed samples from the buffer
cleanup:
    uint16_t temp;
    for (int i = 0; i < handle->configs.sample_size; i++)
    {
        if (circular_buffer_pop(&ctx->input_buffer, &temp) != 0)
        {
            LOG_ERROR("Failed to pop sample from buffer");
            ret = -1;
            goto failed;
        }
    }

    return ret;
failed:
    return ret;
}

static int _calculate_window_offset(fsk_decoder_handle_t *handle, uint16_t *samples, size_t num_samples)
{
    int sample_size = num_samples;
    const int max_offset_range = sample_size / 4;

    // Calculate the offset based off of the offset that gives the greatest power difference
    int max_power_diff = 0;
    int best_offset = 0;
    for (int offset = 0; offset < max_offset_range; offset++)
    {
        float power_0 = 0.0f;
        float power_1 = 0.0f;

        if (goertzel_compute_power(samples + offset, sample_size, handle->configs.freq_0, handle->configs.sample_rate, &power_0) < 0)
        {
            LOG_ERROR("Failed to compute power for frequency %f at offset %d", handle->configs.freq_0, offset);
            continue;
        }

        if (goertzel_compute_power(samples + offset, sample_size, handle->configs.freq_1, handle->configs.sample_rate, &power_1) < 0)
        {
            LOG_ERROR("Failed to compute power for frequency %f at offset %d", handle->configs.freq_1, offset);
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