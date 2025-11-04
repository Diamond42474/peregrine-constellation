#include "encoder.h"

#include "c-logger.h"
#include "bit_unpacker.h"
#include "cobs_encoder.h"

#define COBS_MAX_PAYLOAD_SIZE (254) // Maximum payload size for COBS (code_byte + data + delimiter_byte)

static int _cobs(encoder_handle_t *handle);

int encoder_init(encoder_handle_t *handle)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Handle is NULL");
        return -1;
    }

    handle->preamble_byte = 0xAA; // Default for now
    handle->preamble_length = 1;

    handle->state = ENCODER_UNINITIALIZED;

failed:
    return ret;
}

int encoder_deinit(encoder_handle_t *handle)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Handle is NULL");
        return -1;
    }

    if (handle->state == ENCODER_UNINITIALIZED)
    {
        LOG_WARN("Encoder already not initialized");
        return 0;
    }

    if (circular_buffer_deinit(&handle->input_cb))
    {
        LOG_ERROR("Failed to de-init input buffer");
    }
    if (circular_buffer_deinit(&handle->output_cb))
    {
        LOG_ERROR("Failed to de-init output buffer");
    }

    handle->state = ENCODER_UNINITIALIZED;

failed:
    return ret;
}

int encoder_set_type(encoder_handle_t *handle, encoder_type_e type)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Handle is NULL");
        return -1;
    }

    handle->type = type;

failed:
    return ret;
}

int encoder_set_input_size(encoder_handle_t *handle, size_t buffer_size)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Handle is NULL");
        return -1;
    }

    if (!buffer_size)
    {
        LOG_ERROR("Buffer size must be greater than 0");
        return -1;
    }

    handle->input_cb_size = buffer_size;

    return ret;
}

int encoder_set_output_size(encoder_handle_t *handle, size_t buffer_size)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Handle is NULL");
        return -1;
    }

    if (!buffer_size)
    {
        LOG_ERROR("Buffer size must be greater than 0");
        return -1;
    }

    handle->output_cb_size = buffer_size;

    return ret;
}

int encoder_write(encoder_handle_t *handle, const unsigned char *data, size_t len)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Handle is NULL");
        return -1;
    }

    if (handle->state == ENCODER_UNINITIALIZED)
    {
        LOG_ERROR("Encoder isn't initialized");
        return -1;
    }

    for (int i = 0; i < len; i++)
    {
        if (circular_buffer_push(&handle->input_cb, &data[i]))
        {
            LOG_ERROR("Failed to push to buffer");
            return -1;
        }
    }

    return ret;
}

int encoder_flush(encoder_handle_t *handle)
{
    if (!handle)
    {
        LOG_ERROR("Handle is NULL");
        return -1;
    }

    if (handle->state == ENCODER_WAITING_FOR_FULL_FRAME)
    {
        handle->state = ENCODER_PROCESSING;
    }

    return 0;
}

int encoder_read(encoder_handle_t *handle, bool *bit)
{
    if (!handle)
    {
        LOG_ERROR("Handle is NULL");
        return -1;
    }

    if (handle->state == ENCODER_UNINITIALIZED)
    {
        LOG_ERROR("Encoder isn't initialized");
        return -1;
    }

    if (circular_buffer_pop(&handle->output_cb, bit))
    {
        LOG_ERROR("Failed to pop from buffer");
        return -1;
    }

    return 0;
}

int encoder_task(encoder_handle_t *handle)
{
    if (!handle)
    {
        LOG_ERROR("Handle is NULL");
        return -1;
    }

    switch (handle->state)
    {
    case ENCODER_UNINITIALIZED:

        if (circular_buffer_dynamic_init(&handle->input_cb, sizeof(unsigned char), handle->input_cb_size))
        {
            LOG_ERROR("Failed to init input buffer");
            return -1;
        }
        if (circular_buffer_dynamic_init(&handle->output_cb, sizeof(bool), handle->output_cb_size))
        {
            LOG_ERROR("Failed to init output buffer");
            return -1;
        }
        handle->state = ENCODER_IDLE;
        LOG_DEBUG("Encoder initialized");
        break;
    case ENCODER_IDLE:
        break;
    case ENCODER_WAITING_FOR_FULL_FRAME:
        break;
    case ENCODER_PROCESSING:
        break;
    default:
        LOG_ERROR("Unknown encoder state");
        return -1;
    }
}

bool encoder_busy(encoder_handle_t *handle)
{
    if (!handle)
    {
        LOG_ERROR("Handle is NULL");
        return true;
    }

    return handle->state != ENCODER_IDLE && handle->state != ENCODER_WAITING_FOR_FULL_FRAME;
}

static int _cobs(encoder_handle_t *handle)
{
    return 0;
}