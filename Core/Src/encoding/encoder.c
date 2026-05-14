#include "encoding/encoder.h"

#include "c-logger.h"
#include "utils/bit_unpacker.h"
#include "interface/pconfig.h"

static int _process(encoder_handle_t *handle);
static int _cobs(encoder_handle_t *handle);

int encoder_init(encoder_handle_t *handle)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Handle is NULL");
        return -1;
    }

    bit_unpacker_init(&handle->bit_unpacker);

    handle->preamble[0] = pconfigPREAMBLE_BYTE_1;
    handle->preamble[1] = pconfigPREAMBLE_BYTE_2;
    handle->preamble_length = 2;

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

    if (handle->state == ENCODER_IDLE)
    {
        handle->state = ENCODER_WAITING_FOR_FULL_FRAME;
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

    if (bit_unpacker_pop(&handle->bit_unpacker, &handle->output_cb, bit))
    {
        LOG_ERROR("Failed to unpack bit");
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

        if (circular_buffer_dynamic_init(&handle->input_cb, sizeof(uint8_t), handle->input_cb_size))
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
        // This will be exited when data is written to the input buffer
        break;
    case ENCODER_WAITING_FOR_FULL_FRAME:
        if (circular_buffer_count(&handle->input_cb) >= handle->max_frame_size)
        {
            handle->state = ENCODER_PROCESSING;
        }
        break;
    case ENCODER_PROCESSING:
        if (_process(handle))
        {
            LOG_ERROR("Failed to process encoding");
            return -1;
        }
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

bool encoder_data_available(encoder_handle_t *handle)
{
    if (!handle)
    {
        LOG_ERROR("Handle is NULL");
        return false;
    }

    return circular_buffer_count(&handle->output_cb) > 0 || handle->bit_unpacker.bit_pos < 8;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=
//    PRIVATE FUNCTIONS
// =-=-=-=-=-=-=-=-=-=-=-=-=

static int _process(encoder_handle_t *handle)
{
    int ret = 0;

    // First push preamble
    for (int i = 0; i < handle->preamble_length; i++)
    {
        circular_buffer_push(&handle->output_cb, &handle->preamble[i]); // Place preamble byte
    }

    switch (handle->type)
    {
    case ENCODER_TYPE_NONE:
        // Just push bits directly without encoding
        while (circular_buffer_count(&handle->input_cb) > 0)
        {
            uint8_t byte = 0x00;
            if (circular_buffer_pop(&handle->input_cb, &byte))
            {
                LOG_ERROR("Failed to pop from input buffer");
                return -1;
            }
            if (circular_buffer_push(&handle->output_cb, &byte))
            {
                LOG_ERROR("Failed to push to output buffer");
                return -1;
            }
        }
        handle->state = ENCODER_IDLE; // We can idle after this because we processed all available data
        break;
    default:
        LOG_ERROR("Unknown encoder type");
        return -1;
    }

    return ret;
}