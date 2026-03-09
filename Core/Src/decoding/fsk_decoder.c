#include "decoding/fsk_decoder.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "utils/goertzel.h"
#include "c-logger.h"
#include "utils/circular_buffer.h"

static int _process_samples(fsk_decoder_handle_t *handle, decoder_handle_t *ctx);
static size_t _calculate_window_offset(fsk_decoder_handle_t *handle, const uint16_t *samples, size_t num_samples);
static int _copy_samples_from_circular_buffer(circular_buffer_t *cb, uint16_t *out_samples, size_t num_samples);
static int _discard_samples(circular_buffer_t *cb, size_t num_samples);

int fsk_decoder_init(fsk_decoder_handle_t *handle)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("FSK decoder handle is NULL");
        return -1;
    }

    memset(handle, 0, sizeof(*handle));
    handle->configs.sample_buffer_multiplier = 1;
    handle->symbol_timing_locked = false;
    handle->symbol_timing_offset = 0;
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

    (void)bit_buffer_size;

failed:
    return ret;
}

int fsk_decoder_set_sample_buffer_multiplier(fsk_decoder_handle_t *handle, size_t multiplier)
{
    if (!handle)
    {
        LOG_ERROR("FSK decoder handle is NULL");
        return -1;
    }

    if (multiplier == 0)
    {
        LOG_ERROR("Sample buffer multiplier must be greater than zero");
        return -1;
    }

    handle->configs.sample_buffer_multiplier = multiplier;
    handle->symbol_timing_locked = false;
    handle->symbol_timing_offset = 0;

    return 0;
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
        LOG_ERROR("Invalid sample rate: %d", _sample_rate);
        ret = -1;
        goto failed;
    }

    handle->configs.sample_rate = _sample_rate;
    handle->configs.sample_size = _sample_size;
    handle->symbol_timing_locked = false;
    handle->symbol_timing_offset = 0;

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

int fsk_decoder_reset_symbol_timing(fsk_decoder_handle_t *handle)
{
    if (!handle)
    {
        LOG_ERROR("FSK decoder handle is NULL");
        return -1;
    }

    handle->symbol_timing_locked = false;
    handle->symbol_timing_offset = 0;
    return 0;
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
    {
        size_t required_samples = (size_t)handle->configs.sample_size;

        if (!handle->symbol_timing_locked)
        {
            required_samples *= handle->configs.sample_buffer_multiplier;
        }

        LOG_DEBUG("Decoding samples, current sample buffer size: %zu", circular_buffer_count(&ctx->input_buffer));
        if (circular_buffer_count(&ctx->input_buffer) >= required_samples)
        {
            if (_process_samples(handle, ctx))
            {
                LOG_ERROR("Failed to process samples");
                ret = -1;
                goto failed;
            }
        }
        break;
    }
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

static int _process_samples(fsk_decoder_handle_t *handle, decoder_handle_t *ctx)
{
    int ret = 0;
    float power_0 = 0.0f;
    float power_1 = 0.0f;

    if (!handle->symbol_timing_locked)
    {
        size_t timing_window_size = (size_t)handle->configs.sample_size * handle->configs.sample_buffer_multiplier;
        uint16_t *timing_window = malloc(timing_window_size * sizeof(uint16_t));

        if (!timing_window)
        {
            LOG_ERROR("Failed to allocate timing recovery window of %zu samples", timing_window_size);
            ret = -1;
            goto failed;
        }

        if (_copy_samples_from_circular_buffer(&ctx->input_buffer, timing_window, timing_window_size))
        {
            LOG_ERROR("Failed to copy samples for timing recovery");
            free(timing_window);
            ret = -1;
            goto failed;
        }

        handle->symbol_timing_offset = _calculate_window_offset(handle, timing_window, timing_window_size);
        handle->symbol_timing_locked = true;
        LOG_INFO("FSK symbol timing locked at offset %zu", handle->symbol_timing_offset);

        free(timing_window);

        if (_discard_samples(&ctx->input_buffer, handle->symbol_timing_offset))
        {
            LOG_ERROR("Failed to align to symbol timing offset");
            ret = -1;
            goto failed;
        }
    }

    if (goertzel_compute_power_circular_buff(&ctx->input_buffer,
                                             handle->configs.sample_size,
                                             handle->configs.freq_0,
                                             handle->configs.sample_rate,
                                             &power_0) < 0)
    {
        LOG_ERROR("Failed to compute power for frequency %f", handle->configs.freq_0);
        ret = -1;
        goto failed;
    }

    if (goertzel_compute_power_circular_buff(&ctx->input_buffer,
                                             handle->configs.sample_size,
                                             handle->configs.freq_1,
                                             handle->configs.sample_rate,
                                             &power_1) < 0)
    {
        LOG_ERROR("Failed to compute power for frequency %f", handle->configs.freq_1);
        ret = -1;
        goto failed;
    }

    if (power_0 < handle->configs.power_threshold && power_1 < handle->configs.power_threshold)
    {
        LOG_DEBUG("No significant signal detected (power_0: %f, power_1: %f)", power_0, power_1);
        goto cleanup;
    }

    {
        bool bit = (power_1 > power_0);
        LOG_DEBUG("Bit: %d", bit);

        if (decoder_process_bit(ctx, bit))
        {
            LOG_ERROR("Failed to process decoded bit");
            ret = -1;
            goto failed;
        }
    }

cleanup:
    if (_discard_samples(&ctx->input_buffer, (size_t)handle->configs.sample_size))
    {
        LOG_ERROR("Failed to pop sample from buffer");
        ret = -1;
        goto failed;
    }

failed:
    return ret;
}

static size_t _calculate_window_offset(fsk_decoder_handle_t *handle, const uint16_t *samples, size_t num_samples)
{
    size_t sample_size = (size_t)handle->configs.sample_size;
    size_t max_offset_range = 0;
    size_t best_offset = 0;
    float max_power_diff = -1.0f;

    if (sample_size == 0 || num_samples < sample_size)
    {
        return 0;
    }

    if (num_samples == sample_size)
    {
        return 0;
    }

    max_offset_range = num_samples - sample_size + 1;
    if (max_offset_range > sample_size)
    {
        max_offset_range = sample_size;
    }

    for (size_t offset = 0; offset < max_offset_range; offset++)
    {
        float power_0 = 0.0f;
        float power_1 = 0.0f;

        if (goertzel_compute_power(samples + offset,
                                   sample_size,
                                   handle->configs.freq_0,
                                   handle->configs.sample_rate,
                                   &power_0) < 0)
        {
            LOG_ERROR("Failed to compute power for frequency %f at offset %zu", handle->configs.freq_0, offset);
            continue;
        }

        if (goertzel_compute_power(samples + offset,
                                   sample_size,
                                   handle->configs.freq_1,
                                   handle->configs.sample_rate,
                                   &power_1) < 0)
        {
            LOG_ERROR("Failed to compute power for frequency %f at offset %zu", handle->configs.freq_1, offset);
            continue;
        }

        {
            float power_diff = fabsf(power_1 - power_0);
            if (power_diff > max_power_diff)
            {
                max_power_diff = power_diff;
                best_offset = offset;
            }
        }
    }

    LOG_DEBUG("Best offset found: %zu with power difference: %f", best_offset, max_power_diff);
    return best_offset;
}

static int _copy_samples_from_circular_buffer(circular_buffer_t *cb, uint16_t *out_samples, size_t num_samples)
{
    size_t idx = 0;

    if (!cb || !out_samples)
    {
        return -1;
    }

    if (circular_buffer_count(cb) < num_samples)
    {
        return -1;
    }

    idx = cb->tail;
    for (size_t i = 0; i < num_samples; i++)
    {
        out_samples[i] = *((uint16_t *)cb->buffer + idx);
        idx = (idx + 1) % cb->max;
    }

    return 0;
}

static int _discard_samples(circular_buffer_t *cb, size_t num_samples)
{
    uint16_t sample = 0;

    if (!cb)
    {
        return -1;
    }

    for (size_t i = 0; i < num_samples; i++)
    {
        if (circular_buffer_pop(cb, &sample) != 0)
        {
            return -1;
        }
    }

    return 0;
}
