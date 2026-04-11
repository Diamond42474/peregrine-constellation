#include "decoding/fsk_decoder.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "utils/goertzel.h"
#include "c-logger.h"
#include "utils/circular_buffer.h"

#define QUALITY_THRESHOLD 0.75f

static int _process_samples(fsk_decoder_handle_t *handle, decoder_handle_t *ctx);
static size_t _calculate_window_offset(fsk_decoder_handle_t *handle, decoder_handle_t *ctx);
static float _calculate_quality(fsk_decoder_handle_t *handle, decoder_handle_t *ctx);
static int _update_symbol_timing(fsk_decoder_handle_t *handle, decoder_handle_t *ctx);

/**
 * @brief Initializes the FSK decoder handle with default values.
 *
 * @param handle Pointer to the FSK decoder handle to initialize.
 *
 * @return error code: 0 = success, -1 = failure
 */
int fsk_decoder_init(fsk_decoder_handle_t *handle)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("FSK decoder handle is NULL");
        return -1;
    }

    memset(handle, 0, sizeof(*handle));
    handle->configs.buffer_symbol_count = 1; // Default to no oversampling
    handle->symbol_timing_offset = 0;
    handle->state = FSK_DECODER_STATE_INITIALIZING;

failed:
    return ret;
}

/**
 * @brief Deinitializes the FSK decoder handle, freeing any resources if necessary.
 *
 * @param handle Pointer to the FSK decoder handle to deinitialize.
 *
 * @return error code: 0 = success, -1 = failure
 */
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

/**
 * @brief Sets the symbol sample size and buffer symbol count for the FSK decoder.
 *
 * @note Having multiple symbols buffered can help with timing recovery and improve decoding performance,
 *       but it also increases latency and memory usage.
 *
 * @param handle Pointer to the FSK decoder handle.
 * @param _symbol_sample_size The number of ADC samples that correspond to one symbol (must be greater than 0).
 * @param _buffer_symbol_count The number of symbols to buffer for processing (must be greater than 0).
 *
 * @return error code: 0 = success, -1 = failure
 */
int fsk_decoder_set_symbol_sample_size(fsk_decoder_handle_t *handle, size_t _symbol_sample_size, size_t _buffer_symbol_count)
{
    if (!handle)
    {
        LOG_ERROR("FSK decoder handle is NULL");
        return -1;
    }

    if (_symbol_sample_size == 0)
    {
        LOG_ERROR("Symbol sample size must be greater than 0");
        return -1;
    }

    if (_buffer_symbol_count == 0)
    {
        LOG_ERROR("Buffer symbol count must be greater than 0");
        return -1;
    }

    handle->configs.symbol_sample_size = _symbol_sample_size;
    handle->configs.buffer_symbol_count = _buffer_symbol_count;

    return 0;
}

/**
 * @brief Sets the sample rate used for frequency calculations
 *
 * @param handle Pointer to the FSK decoder handle.
 * @param sample_rate The sample rate of the ADC in Hz (must be greater than 0).
 *
 * @return error code: 0 = success, -1 = failure
 */
int fsk_decoder_set_sample_rate(fsk_decoder_handle_t *handle, int sample_rate)
{
    if (!handle)
    {
        LOG_ERROR("FSK decoder handle is NULL");
        return -1;
    }

    if (sample_rate <= 0)
    {
        LOG_ERROR("Sample rate must be greater than 0");
        return -1;
    }

    handle->configs.sample_rate = sample_rate;

    return 0;
}

/**
 * @brief Sets the frequencies representing bit 0 and bit 1 for the FSK decoder.
 *
 * @param handle Pointer to the FSK decoder handle.
 * @param _freq_0 The frequency representing bit 0 in Hz.
 * @param _freq_1 The frequency representing bit 1 in Hz.
 *
 * @return error code: 0 = success, -1 = failure
 */
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

/**
 * @brief Sets the power threshold for detecting bits in the FSK decoder.
 *
 * @note The FSK decoder uses the Goertzel algorithm to compute the power at a specifc frequency.
 *       If the power at both frequencies is below this threshold, the decoder will consider it as no signal.
 *
 * @param handle Pointer to the FSK decoder handle.
 * @param _threshold The power threshold for detecting bits (must be greater than 0).
 *
 * @return error code: 0 = success, -1 = failure
 */
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

/**
 * @brief Tells the FSK decoder to do symbol timing recovery.
 *
 * @param handle Pointer to the FSK decoder handle.
 *
 * @return error code: 0 = success, -1 = failure
 */
int fsk_decoder_reset_symbol_timing(fsk_decoder_handle_t *handle)
{
    if (!handle)
    {
        LOG_ERROR("FSK decoder handle is NULL");
        return -1;
    }

    handle->state = FSK_DECODER_RECOVERING_TIMING;
    handle->symbol_timing_offset = 0;
    return 0;
}

/**
 * @brief Main task function for the FSK decoder. This should be called periodically to process incoming samples and decode bits.
 *
 * @param handle Pointer to the FSK decoder handle.
 * @param ctx Pointer to the main decoder context, which contains the input sample buffer and output bit buffer.
 *
 * @return error code: 0 = success, -1 = failure
 */
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
            handle->state = FSK_DECODER_WAITING_FOR_SIGNAL;
        }
        break;
    case FSK_DECODER_WAITING_FOR_SIGNAL:
        if (handle->signal_detected)
        {
            LOG_DEBUG("Signal already detected, skipping quality check");
            handle->state = FSK_DECODER_RECOVERING_TIMING;
            break;
        }

        if (circular_buffer_count(&ctx->input_buffer) >= (size_t)handle->configs.symbol_sample_size * handle->configs.buffer_symbol_count)
        {
            float quality = _calculate_quality(handle, ctx);
            LOG_DEBUG("Calculated signal quality: %f", quality);
            if (quality >= QUALITY_THRESHOLD)
            {
                LOG_INFO("Signal detected with quality %f, starting timing recovery", quality);
                handle->signal_detected = true;
                handle->state = FSK_DECODER_RECOVERING_TIMING;
            }
            else
            {
                LOG_DEBUG("Signal quality %f is below threshold %f, continuing to wait", quality, QUALITY_THRESHOLD);
                // Discard some samples to avoid getting stuck on the same low-quality signal
                for (int i = 0; i < (int)handle->configs.symbol_sample_size; i++)
                {
                    circular_buffer_remove(&ctx->input_buffer); // remove 1 item
                }
            }
        }
        break;
    case FSK_DECODER_RECOVERING_TIMING:
        if (circular_buffer_count(&ctx->input_buffer) >= (size_t)handle->configs.symbol_sample_size * handle->configs.buffer_symbol_count)
        {
            if (_update_symbol_timing(handle, ctx))
            {
                LOG_ERROR("Failed to recover symbol timing");
                ret = -1;
                goto failed;
            }
            handle->state = FSK_DECODER_STATE_DECODING;
        }
        break;
    case FSK_DECODER_STATE_DECODING:
    {
        size_t required_samples = (size_t)handle->configs.symbol_sample_size;

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

bool fsk_decoder_busy(fsk_decoder_handle_t *handle, decoder_handle_t *ctx)
{
    if (!handle)
    {
        LOG_ERROR("FSK decoder handle is NULL");
        return false;
    }

    return circular_buffer_count(&ctx->input_buffer) >= (size_t)handle->configs.symbol_sample_size * handle->configs.buffer_symbol_count;
}

static int _update_symbol_timing(fsk_decoder_handle_t *handle, decoder_handle_t *ctx)
{
    int ret = 0;

    if (!handle || !ctx)
    {
        LOG_ERROR("FSK decoder handle or context is NULL");
        return -1;
    }

    if (circular_buffer_count(&ctx->input_buffer) < (size_t)handle->configs.symbol_sample_size * handle->configs.buffer_symbol_count)
    {
        LOG_WARN("Not enough samples in circular buffer to update symbol timing");
        return 0;
    }

    size_t offset = _calculate_window_offset(handle, ctx);
    if (offset == (size_t)-1)
    {
        LOG_ERROR("Failed to calculate window offset for symbol timing recovery");
        return -1;
    }

    handle->symbol_timing_offset = offset;
    for (int i = 0; i < offset; i++)
    {
        circular_buffer_remove(&ctx->input_buffer);
    }
    LOG_INFO("Symbol timing offset updated: %zu", offset);

    return 0;
}

static int _process_samples(fsk_decoder_handle_t *handle, decoder_handle_t *ctx)
{
    int ret = 0;
    float power_0 = 0.0f;
    float power_1 = 0.0f;

    if (goertzel_compute_power_circular_buff(&ctx->input_buffer, handle->configs.symbol_sample_size, handle->configs.freq_0, handle->configs.sample_rate, &power_0) < 0)
    {
        LOG_ERROR("Failed to compute power for frequency %f", handle->configs.freq_0);
        ret = -1;
        goto failed;
    }

    if (goertzel_compute_power_circular_buff(&ctx->input_buffer, handle->configs.symbol_sample_size, handle->configs.freq_1, handle->configs.sample_rate, &power_1))
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
    // Remove the samples corresponding to one symbol from the input buffer
    for (int i = 0; i < (int)handle->configs.symbol_sample_size; i++)
    {
        circular_buffer_remove(&ctx->input_buffer); // remove 1 item
    }

failed:
    return ret;
}

static size_t _calculate_window_offset(fsk_decoder_handle_t *handle, decoder_handle_t *ctx)
{
    LOG_INFO("Finding optimal window offset for symbol timing recovery");
    size_t symbol_sample_size = (size_t)handle->configs.symbol_sample_size;
    size_t max_offset_range = 0;
    size_t best_offset = 0;
    float max_power_diff = -1.0f;

    size_t num_samples = circular_buffer_count(&ctx->input_buffer);

    circular_buffer_t temp_cb = ctx->input_buffer;

    if (symbol_sample_size == 0 || num_samples < symbol_sample_size * handle->configs.buffer_symbol_count)
    {
        LOG_ERROR("Not enough samples in circular buffer to calculate window offset (num_samples: %zu, required: %zu)", num_samples, symbol_sample_size * handle->configs.buffer_symbol_count);
        return -1;
    }

    if (num_samples == symbol_sample_size)
    {
        LOG_WARN("Number of samples equals symbol sample size, no offset calculation needed");
        return 0;
    }

    max_offset_range = num_samples - symbol_sample_size + 1;
    for (size_t offset = 0; offset < num_samples - symbol_sample_size + 1; offset++)
    {
        float power_0 = 0.0f;
        float power_1 = 0.0f;

        if (goertzel_compute_power_circular_buff(&temp_cb, symbol_sample_size, handle->configs.freq_0, handle->configs.sample_rate, &power_0))
        {
            LOG_ERROR("Not enough samples in circular buffer for offset %zu", offset);
            break;
        }

        if (goertzel_compute_power_circular_buff(&temp_cb, symbol_sample_size, handle->configs.freq_1, handle->configs.sample_rate, &power_1))
        {
            LOG_ERROR("Not enough samples in circular buffer for offset %zu", offset);
            break;
        }

        circular_buffer_remove(&temp_cb); // Move the window by one sample for the next offset calculation

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

static float _calculate_quality(fsk_decoder_handle_t *handle, decoder_handle_t *ctx)
{
    float power_0 = 0.0f;
    float power_1 = 0.0f;
    float sum_diff = 0.0f;
    float sum_power = 0.0f;

    circular_buffer_t temp_cb = ctx->input_buffer;

    LOG_DEBUG("Calculating signal quality with %zu buffered symbols", handle->configs.buffer_symbol_count);
    for (int i = 0; i < (int)handle->configs.buffer_symbol_count; i++)
    {
        if (goertzel_compute_power_circular_buff(&temp_cb, handle->configs.symbol_sample_size, handle->configs.freq_0, handle->configs.sample_rate, &power_0))
        {
            LOG_WARN("Not enough samples in circular buffer for quality calculation at symbol %d", i);
            break;
        }

        if (goertzel_compute_power_circular_buff(&temp_cb, handle->configs.symbol_sample_size, handle->configs.freq_1, handle->configs.sample_rate, &power_1))
        {
            LOG_WARN("Not enough samples in circular buffer for quality calculation at symbol %d", i);
            break;
        }

        sum_diff += fabsf(power_1 - power_0);
        sum_power += power_1 + power_0;

        for (int j = 0; j < (int)handle->configs.symbol_sample_size; j++)
        {
            circular_buffer_remove(&temp_cb); // Move the window by one symbol for the next calculation
        }
    }

    float quality = sum_diff / (sum_power + 1e-6f); // Add small value to avoid division by zero
    LOG_DEBUG("Signal quality: %f", quality);
    return quality;
}
