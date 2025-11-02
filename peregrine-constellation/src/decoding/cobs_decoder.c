#include "cobs_decoder.h"

#include <string.h>
#include <stdbool.h>

#include "c-logger.h"

#define FRAME_BUFFER_SIZE 256

/**
 * @brief Message structure for decoded COBS frames.
 */
typedef struct
{
    unsigned char frame_buffer[FRAME_BUFFER_SIZE]; ///< Buffer to hold incoming frame data
    size_t length;                                 ///< Length of valid data in frame_buffer
} message_t;

static void _process(cobs_decoder_t *handle);
static void _cobs(cobs_decoder_t *handle, unsigned char byte);
static int _reset(cobs_decoder_t *handle);

int cobs_decoder_init(cobs_decoder_t *handle)
{
    if (!handle)
    {
        LOG_ERROR("COBS Decoder handle is NULL");
        return -1;
    }

    handle->write_index = 0;
    handle->code = 0;
    handle->remaining = 0;
    handle->state = COBS_DECODER_STATE_INITIALIZING;

    return 0;
}

int cobs_decoder_deinit(cobs_decoder_t *handle)
{
    if (!handle)
    {
        LOG_ERROR("COBS Decoder handle is NULL");
        return -1;
    }

    if (handle->state == COBS_DECODER_STATE_UNINITIALIZED || handle->state == COBS_DECODER_STATE_INITIALIZING)
    {
        LOG_WARN("COBS Decoder is already uninitialized");
        return 0;
    }

    // Free circular buffers
    if (circular_buffer_deinit(&handle->input_cb))
    {
        LOG_ERROR("Failed to deinitialize COBS Decoder frame buffer");
        return -1;
    }
    if (circular_buffer_deinit(&handle->output_cb))
    {
        LOG_ERROR("Failed to deinitialize COBS Decoder output buffer");
        return -1;
    }

    handle->state = COBS_DECODER_STATE_UNINITIALIZED;

    return 0;
}

int cobs_decoder_set_input_buffer_size(cobs_decoder_t *handle, size_t size)
{
    if (!handle)
    {
        LOG_ERROR("COBS Decoder handle is NULL");
        return -1;
    }

    handle->input_buffer_size = size;

    return 0;
}

int cobs_decoder_set_output_buffer_size(cobs_decoder_t *handle, size_t size)
{
    if (!handle)
    {
        LOG_ERROR("COBS Decoder handle is NULL");
        return -1;
    }

    handle->output_buffer_size = size;

    return 0;
}

int cobs_decoder_process(cobs_decoder_t *handle, unsigned char byte)
{
    if (!handle)
    {
        LOG_ERROR("COBS Decoder handle is NULL");
        return -1;
    }

    if (handle->state == COBS_DECODER_STATE_UNINITIALIZED || handle->state == COBS_DECODER_STATE_INITIALIZING)
    {
        LOG_ERROR("COBS Decoder is uninitialized");
        return -1;
    }

    if (circular_buffer_push(&handle->input_cb, &byte))
    {
        LOG_ERROR("Failed to push byte to COBS Decoder frame buffer");
        return -1;
    }

    handle->state = COBS_DECODER_STATE_DECODING;

    return 0;
}

bool cobs_decoder_has_message(cobs_decoder_t *handle)
{
    if (!handle)
    {
        LOG_ERROR("COBS Decoder handle is NULL");
        return false;
    }

    if (handle->state == COBS_DECODER_STATE_UNINITIALIZED || handle->state == COBS_DECODER_STATE_INITIALIZING)
    {
        LOG_ERROR("COBS Decoder is uninitialized");
        return false;
    }

    if (!circular_buffer_is_empty(&handle->output_cb))
    {
        return true;
    }

    return false;
}

int cobs_decoder_get_message(cobs_decoder_t *handle, unsigned char *buffer, size_t buffer_len, size_t *message_len)
{
    if (!handle || !buffer || buffer_len == 0 || !message_len)
    {
        LOG_ERROR("Invalid parameters to get COBS message");
        return -1;
    }

    if (handle->state == COBS_DECODER_STATE_UNINITIALIZED || handle->state == COBS_DECODER_STATE_INITIALIZING)
    {
        LOG_ERROR("COBS Decoder is uninitialized");
        return -1;
    }

    if (circular_buffer_is_empty(&handle->output_cb))
    {
        LOG_WARN("No COBS messages available to read");
        return -1; // No messages available
    }

    message_t msg;
    if (circular_buffer_pop(&handle->output_cb, &msg))
    {
        LOG_ERROR("Failed to pop COBS message from output buffer");
        return -1;
    }

    if (msg.length > buffer_len)
    {
        LOG_ERROR("Provided buffer is too small for COBS message");
        return -1;
    }

    memcpy(buffer, msg.frame_buffer, msg.length);
    *message_len = msg.length;

    return 0;
}

void cobs_decoder_task(cobs_decoder_t *handle)
{
    if (!handle)
    {
        LOG_ERROR("COBS Decoder handle is NULL");
        return;
    }

    switch (handle->state)
    {
    case COBS_DECODER_STATE_UNINITIALIZED:
        LOG_ERROR("COBS Decoder is uninitialized");
        break;
    case COBS_DECODER_STATE_INITIALIZING:

        // Initialize input buffer with specified size
        if (circular_buffer_dynamic_init(&handle->input_cb, sizeof(unsigned char), handle->input_buffer_size))
        {
            LOG_ERROR("Failed to initialize COBS Decoder input buffer");
            return;
        }

        // Initialize output fuffer with specified size
        if (circular_buffer_dynamic_init(&handle->output_cb, sizeof(message_t), handle->output_buffer_size))
        {
            LOG_ERROR("Failed to initialize COBS Decoder output buffer");
            return;
        }
        LOG_INFO("COBS Decoder initialized");
        handle->state = COBS_DECODER_STATE_IDLE;
        break;
    case COBS_DECODER_STATE_IDLE:
        // Idle state, waiting for data
        if (!circular_buffer_is_empty(&handle->input_cb))
        {
            handle->state = COBS_DECODER_STATE_DECODING;
        }
        break;
    case COBS_DECODER_STATE_DECODING:
        // Decoding state, process data if available
        while (!circular_buffer_is_empty(&handle->input_cb))
        {
            _process(handle);
        }
        handle->state = COBS_DECODER_STATE_IDLE;
        break;
    default:
        LOG_ERROR("Unknown COBS Decoder state");
        handle->state = COBS_DECODER_STATE_UNINITIALIZED;
        break;
    }
}

bool cobs_decoder_busy(cobs_decoder_t *handle)
{
    if (!handle)
    {
        LOG_ERROR("COBS Decoder handle is NULL");
        return false;
    }

    return handle->state != COBS_DECODER_STATE_IDLE;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//      PRIVATE FUNCTIONS
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void _process(cobs_decoder_t *handle)
{
    // TODO: Handle starting reset condition
    // Something like if you see 2x 0xAA bytes in a row, reset the decoder

    unsigned char byte;
    if (circular_buffer_pop(&handle->input_cb, &byte))
    {
        return;
    }

    _cobs(handle, byte);
    return;
}

static void _cobs(cobs_decoder_t *handle, unsigned char byte)
{
    LOG_INFO("COBS Decoder Input: %02X", byte);

    if (handle->code == 0)
    {
        // Start of a new block: code byte indicates how many bytes follow
        if (byte == 0)
        {
            LOG_INFO("COBS Decoder: Zero byte received, resetting decoder");

            // End of frame delimiter
            if (handle->write_index > 0)
            {
                message_t msg;
                memcpy(msg.frame_buffer, handle->frame_buffer, handle->write_index);
                msg.length = handle->write_index;
            }
            else
            {
                LOG_ERROR("COBS Decoder: No data to process, resetting decoder");
            }
            // Reset for next frame
            _reset(handle);
            return;
        }

        handle->code = byte;
        handle->remaining = handle->code - 1;

        if (handle->write_index + handle->remaining >= FRAME_BUFFER_SIZE)
        {
            LOG_ERROR("COBS Decoder: Frame buffer overflow, resetting decoder");
            // Overflow - reset
            _reset(handle);
            return;
        }

        return;
    }

    // Copy next data byte
    handle->frame_buffer[handle->write_index++] = byte;
    handle->remaining--;

    if (handle->remaining == 0)
    {
        // If code < 0xFF, insert a zero byte after this block (except if it's the last block before delimiter)
        if (handle->code < 0xFF && handle->write_index < FRAME_BUFFER_SIZE)
        {
            handle->frame_buffer[handle->write_index++] = 0;
        }
        handle->code = 0; // Ready for next code byte
    }
}

static int _reset(cobs_decoder_t *handle)
{
    if (!handle)
    {
        LOG_ERROR("COBS Decoder handle is NULL");
        return -1;
    }

    if (handle->state == COBS_DECODER_STATE_UNINITIALIZED || handle->state == COBS_DECODER_STATE_INITIALIZING)
    {
        LOG_ERROR("COBS Decoder is uninitialized");
        return -1;
    }

    handle->code = 0;
    handle->remaining = 0;
    handle->write_index = 0;

    return 0;
}