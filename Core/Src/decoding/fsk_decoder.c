#include "decoding/fsk_decoder.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "utils/goertzel.h"
#include "c-logger.h"
#include "utils/circular_buffer.h"
#include "dsp/filters.h"

static int _process_samples(fsk_decoder_handle_t *handle, decoder_handle_t *ctx);
static size_t _calculate_window_offset(fsk_decoder_handle_t *handle, decoder_handle_t *ctx);
static float _calculate_quality(fsk_decoder_handle_t *handle, decoder_handle_t *ctx);
static int _update_symbol_timing(fsk_decoder_handle_t *handle, decoder_handle_t *ctx);
static int _process_sample(uint16_t sample, fsk_decoder_handle_t *handle, decoder_handle_t *ctx);
static int _process_afsk_samples(fsk_decoder_handle_t *handle, decoder_handle_t *ctx);

static env_metric_t env_metric;

static biquad_t bp1200[4] = {
    {.c = {9.69336931e-08f, 1.93867386e-07f, 9.69336931e-08f,
           -1.95299489f, 9.65474572e-01f},
     .x1 = 0,
     .x2 = 0,
     .y1 = 0,
     .y2 = 0},
    {.c = {1.0f, 2.0f, 1.0f,
           -1.96000340f, 9.69631439e-01f},
     .x1 = 0,
     .x2 = 0,
     .y1 = 0,
     .y2 = 0},
    {.c = {1.0f, -2.0f, 1.0f,
           -1.96926730f, 9.84359819e-01f},
     .x1 = 0,
     .x2 = 0,
     .y1 = 0,
     .y2 = 0},
    {.c = {1.0f, -2.0f, 1.0f,
           -1.98039793f, 9.88510861e-01f},
     .x1 = 0,
     .x2 = 0,
     .y1 = 0,
     .y2 = 0}};

static biquad_t bp2200[4] = {
    {.c = {9.69336931e-08f, 1.93867386e-07f, 9.69336931e-08f,
           -1.92625778f, 9.66447109e-01f},
     .x1 = 0,
     .x2 = 0,
     .y1 = 0,
     .y2 = 0},
    {.c = {1.0f, 2.0f, 1.0f,
           -1.93366956f, 9.68655698e-01f},
     .x1 = 0,
     .x2 = 0,
     .y1 = 0,
     .y2 = 0},
    {.c = {1.0f, -2.0f, 1.0f,
           -1.94056057f, 9.85315149e-01f},
     .x1 = 0,
     .x2 = 0,
     .y1 = 0,
     .y2 = 0},
    {.c = {1.0f, -2.0f, 1.0f,
           -1.95553640f, 9.87552432e-01f},
     .x1 = 0,
     .x2 = 0,
     .y1 = 0,
     .y2 = 0}};

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
    handle->signal_detected = false;
    handle->edge_detected = false;
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
        float gap = 200;
        // init_bandpass_4th(handle->configs.freq_0 - gap, handle->configs.freq_0 + gap, handle->configs.sample_rate, &bp1200_1, &bp1200_2);
        // init_bandpass_4th(handle->configs.freq_1 - gap, handle->configs.freq_1 + gap, handle->configs.sample_rate, &bp2200_1, &bp2200_2);
        env_metric_init(&env_metric, (float)handle->configs.sample_rate, 0.003f);
        env_metric.alpha = 0.005f;                                                // Smoothing factor for energy metric, adjust as needed
        handle->half_symbol_sample_size = handle->configs.symbol_sample_size / 2; // Used for timing recovery
        handle->prev_metric = 0.0f;
        LOG_INFO("FSK decoder initialized with symbol_sample_size=%d, buffer_symbol_count=%d, \nsample_rate=%d, freq_0=%.1f, freq_1=%.1f, power_threshold=%.2f",
                 handle->configs.symbol_sample_size,
                 handle->configs.buffer_symbol_count,
                 handle->configs.sample_rate,
                 handle->configs.freq_0,
                 handle->configs.freq_1,
                 handle->configs.power_threshold);
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
        if (circular_buffer_count(&ctx->input_buffer) < 1)
        {
            handle->state = FSK_DECODER_STATE_IDLE;
            break;
        }
        _process_afsk_samples(handle, ctx);
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

int _process_sample(uint16_t sample, fsk_decoder_handle_t *handle, decoder_handle_t *ctx)
{
    // Turn DC 12bit sample into float centered around 0
    float normalized_sample = ((float)sample - 2048.0f) / 2048.0f;

    float filtered_1200 = process_sos(bp1200, 4, normalized_sample);
    float filtered_2200 = process_sos(bp2200, 4, normalized_sample);

    float metric = env_metric_process(&env_metric, filtered_1200, filtered_2200);
    if (metric >= handle->configs.power_threshold && handle->prev_metric < handle->configs.power_threshold)
    {
        LOG_DEBUG("Rising edge detected: metric = %f", metric);
        handle->edge_detected = true;
        handle->metric_ticker = 0; // Reset timer on rising edge
    }
    else if (metric < -handle->configs.power_threshold && handle->prev_metric >= -handle->configs.power_threshold)
    {
        LOG_DEBUG("Falling edge detected: metric = %f", metric);
        handle->edge_detected = true;
        handle->metric_ticker = 0; // Reset timer on falling edge
    }

    handle->prev_metric = metric;

    if ((handle->metric_ticker >= handle->half_symbol_sample_size) && (handle->edge_detected || handle->signal_detected))
    {
        if (metric >= handle->configs.power_threshold)
        {
            LOG_DEBUG("1");
            handle->signal_detected = true;
            if (decoder_process_bit(ctx, true))
            {
                LOG_ERROR("Failed to process decoded bit");
                return -1;
            }
        }
        else if (metric < -handle->configs.power_threshold)
        {
            LOG_DEBUG("0");
            handle->signal_detected = true;
            if (decoder_process_bit(ctx, false))
            {
                LOG_ERROR("Failed to process decoded bit");
                return -1;
            }
        }
        else
        {
            handle->signal_detected = false;
        }

        handle->edge_detected = false;
        handle->metric_ticker = -handle->half_symbol_sample_size; // Set to negative so we wait a full symbol period before measuring again
    }
    handle->metric_ticker++;

    return 0;
}

int _process_afsk_samples(fsk_decoder_handle_t *handle, decoder_handle_t *ctx)
{
    int ret = 0;

    if (!handle || !ctx)
    {
        LOG_ERROR("FSK decoder handle or context is NULL");
        return -1;
    }

    uint16_t sample;
    static int count = 0;
    while (circular_buffer_is_empty(&ctx->input_buffer) == false)
    {
        if (circular_buffer_pop(&ctx->input_buffer, &sample))
        {
            LOG_ERROR("Failed to pop sample from circular buffer");
            ret = -1;
            goto failed;
        }
        _process_sample(sample, handle, ctx);
    }

failed:
    return ret;
}
