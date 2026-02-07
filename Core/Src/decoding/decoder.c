/**
 * @file decoder.c
 *
 * @author Diamond42474
 *
 * The decoder handles the pipeline from taking in sine wave samples
 * to generating frames.
 */
#include "decoding/decoder.h"

#include "c-logger.h"
#include "decoding/fsk_decoder.h"
#include "decoding/byte_assembler.h"

static void _handle_sub_tasks(decoder_handle_t *handle);

/**
 * @brief Initializes decoder
 *
 * @param handle pointer to decoder handle
 *
 * @return error code: 0 = successful, -1 = failed
 */
int decoder_init(decoder_handle_t *handle)
{
    if (!handle)
    {
        LOG_ERROR("Decoder handle is NULL");
        return -1;
    }

    handle->bit_decoder = BIT_DECODER_NONE;
    handle->byte_decoder = BYTE_DECODER_NONE;

    handle->bit_decoder_handle = NULL;
    handle->byte_decoder_handle = NULL;

    handle->input_buffer_size = 0;
    handle->output_buffer_size = 0;

    if (packet_decoder_init(&handle->packet_decoder, handle))
    {
        LOG_ERROR("Failed to initialize packet decoder");
        return -1;
    }

    handle->state = DECODER_STATE_INITIALIZING;

    return 0;
}

/**
 * @brief Deinitializes decoder
 *
 * @note currently doesn't deinit sub-modules
 *
 * @param handle pointer to decoder handle
 *
 * @return error code: 0 = successful, -1 = failed
 */
int decoder_deinit(decoder_handle_t *handle)
{
    if (!handle)
    {
        LOG_ERROR("Decoder handle is NULL");
        return -1;
    }

    if (handle->state == DECODER_STATE_UNINITIALIZED)
    {
        LOG_WARN("Decoder is already uninitialized");
        return 0;
    }

    handle->bit_decoder = BIT_DECODER_NONE;
    handle->byte_decoder = BYTE_DECODER_NONE;

    handle->bit_decoder_handle = NULL;
    handle->byte_decoder_handle = NULL;

    // TODO: Deinit sub modules

    handle->state = DECODER_STATE_UNINITIALIZED;

    return 0;
}

int decoder_set_byte_decoder(decoder_handle_t *handle, byte_decoder_e type, void *byte_decoder_handle)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Decoder handle is NULL");
        return -1;
    }

    if (!byte_decoder_handle)
    {
        LOG_ERROR("Byte decoder handle is NULL");
        ret = -1;
        goto failed;
    }

    handle->byte_decoder = type;
    handle->byte_decoder_handle = byte_decoder_handle;

failed:
    return ret;
}

int decoder_set_bit_decoder(decoder_handle_t *handle, bit_decoder_e type, void *bit_decoder_handle)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Decoder handle is NULL");
        return -1;
    }

    if (!bit_decoder_handle)
    {
        LOG_ERROR("FSK decoder handle is NULL");
        ret = -1;
        goto failed;
    }

    handle->bit_decoder = type;
    handle->bit_decoder_handle = bit_decoder_handle;

failed:
    return ret;
}

int decoder_set_input_buffer_size(decoder_handle_t *handle, size_t size)
{
    if (!handle)
    {
        LOG_ERROR("Decoder handle is NULL");
        return -1;
    }

    handle->input_buffer_size = size;
    return 0;
}

int decoder_set_output_buffer_size(decoder_handle_t *handle, size_t size)
{
    if (!handle)
    {
        LOG_ERROR("Decoder handle is NULL");
        return -1;
    }

    handle->output_buffer_size = size;
    return 0;
}

int decoder_task(decoder_handle_t *handle)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Decoder handle is NULL");
        return -1;
    }

    switch (handle->state)
    {
    case DECODER_STATE_UNINITIALIZED:
        LOG_ERROR("Decoder is uninitialized");
        ret = -1;
        goto failed;
        break;
    case DECODER_STATE_INITIALIZING:
        LOG_INFO("Initializing decoder...");

        // Input buffer takes in 12-bit samples as uint16_t
        if (circular_buffer_dynamic_init(&handle->input_buffer, sizeof(uint16_t), handle->input_buffer_size))
        {
            LOG_ERROR("Failed to initialize input buffer");
            ret = -1;
            handle->state = DECODER_STATE_UNINITIALIZED;
            goto failed;
        }

        // Output buffer holds decoded bytes
        if (circular_buffer_dynamic_init(&handle->output_buffer, sizeof(packet_t), handle->output_buffer_size))
        {
            LOG_ERROR("Failed to initialize output buffer");
            ret = -1;
            handle->state = DECODER_STATE_UNINITIALIZED;
            goto failed;
        }

        handle->state = DECODER_STATE_IDLE;
        break;
    case DECODER_STATE_IDLE:
        // Check if subtasks need to process
        // Adding data to input buffer should trigger processing
        _handle_sub_tasks(handle);
        break;
    case DECODER_STATE_PROCESSING:
        _handle_sub_tasks(handle);
        if (circular_buffer_is_empty(&handle->input_buffer))
        {
            handle->state = DECODER_STATE_IDLE;
        }
        break;
    default:
        LOG_ERROR("Unknown decoder state");
        handle->state = DECODER_STATE_UNINITIALIZED;
        ret = -1;
        goto failed;
        break;
    }

failed:
    return ret;
}

int decoder_process_samples(decoder_handle_t *handle, const uint16_t *samples, size_t num_samples)
{
    int ret = 0;

    if (!handle || !samples || num_samples == 0)
    {
        LOG_ERROR("Invalid arguments to decoder_process_samples");
        return -1;
    }

    if (handle->state == DECODER_STATE_UNINITIALIZED || handle->state == DECODER_STATE_INITIALIZING)
    {
        LOG_ERROR("Decoder is uninitialized");
        return -1;
    }

    for (size_t i = 0; i < num_samples; i++)
    {
        if (circular_buffer_push(&handle->input_buffer, (uint16_t *)&samples[i]))
        {
            LOG_ERROR("Failed to push sample to input buffer");
            ret = -1;
            goto failed;
        }
    }

    handle->state = DECODER_STATE_PROCESSING;

failed:
    return ret;
}

int decoder_process_bit(decoder_handle_t *handle, bool bit)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Decoder handle is NULL");
        return -1;
    }

    if (handle->state == DECODER_STATE_UNINITIALIZED || handle->state == DECODER_STATE_INITIALIZING)
    {
        LOG_ERROR("Decoder is uninitialized");
        return -1;
    }

    // Push bit to byte decoder
    switch (handle->byte_decoder)
    {
    case BYTE_DECODER_NONE:
        LOG_ERROR("No byte decoder set");
        ret = -1;
        goto failed;
        break;
    case BYTE_DECODER_BIT_STUFFING:
        if (byte_assembler_process_bit((byte_assembler_handle_t *)handle->byte_decoder_handle, handle, bit))
        {
            LOG_ERROR("Failed to process bit in byte assembler");
            ret = -1;
            goto failed;
        }
        break;
    default:
        LOG_ERROR("Unknown byte decoder type");
        ret = -1;
        goto failed;
        break;
    }

failed:
    return ret;
}

int decoder_process_byte(decoder_handle_t *handle, unsigned char byte)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Decoder handle is NULL");
        return -1;
    }

    if (handle->state == DECODER_STATE_UNINITIALIZED || handle->state == DECODER_STATE_INITIALIZING)
    {
        LOG_ERROR("Decoder is uninitialized");
        return -1;
    }

    if (packet_decoder_process_byte(&handle->packet_decoder, byte))
    {
        LOG_ERROR("Failed to process byte in packet decoder");
        ret = -1;
        goto failed;
    }

failed:
    return ret;
}

int decoder_process_packet(decoder_handle_t *handle, packet_t *packet)
{
    int ret = 0;

    if (!handle || !packet)
    {
        LOG_ERROR("Invalid arguments to decoder_process_packet");
        return -1;
    }

    if (handle->state == DECODER_STATE_UNINITIALIZED || handle->state == DECODER_STATE_INITIALIZING)
    {
        LOG_ERROR("Decoder is uninitialized");
        return -1;
    }

    // Push packet to output buffer
    if (circular_buffer_push(&handle->output_buffer, packet))
    {
        LOG_ERROR("Failed to push packet to output buffer");
        ret = -1;
        goto failed;
    }

failed:
    return ret;
}

bool decoder_has_packet(decoder_handle_t *handle)
{
    if (!handle)
    {
        LOG_ERROR("Decoder handle is NULL");
        return false;
    }

    if (handle->state == DECODER_STATE_UNINITIALIZED || handle->state == DECODER_STATE_INITIALIZING)
    {
        LOG_ERROR("Decoder is uninitialized");
        return false;
    }

    if (!circular_buffer_is_empty(&handle->output_buffer))
    {
        return true;
    }

    return false;
}

int decoder_get_packet(decoder_handle_t *handle, packet_t *packet)
{
    int ret = 0;

    if (!handle || !packet)
    {
        LOG_ERROR("Invalid arguments to decoder_get_packet");
        return -1;
    }

    if (handle->state == DECODER_STATE_UNINITIALIZED || handle->state == DECODER_STATE_INITIALIZING)
    {
        LOG_ERROR("Decoder is uninitialized");
        return -1;
    }

    if (circular_buffer_is_empty(&handle->output_buffer))
    {
        LOG_WARN("No packets available in output buffer");
        ret = -1;
        goto failed;
    }

    // Pop packet from output buffer
    if (circular_buffer_pop(&handle->output_buffer, packet))
    {
        LOG_ERROR("Failed to pop packet from output buffer");
        ret = -1;
        goto failed;
    }

failed:
    return ret;
}

/**
 * @brief Checks if any sub-modules are busy
 *
 * @param handle pointer to decoder handle
 *
 * @return if decoder is busy or not
 */
bool decoder_busy(decoder_handle_t *handle)
{
    if (!handle)
    {
        LOG_ERROR("Handle is NULL");
        return false;
    }

    return handle->state != DECODER_STATE_IDLE;
}

static void _handle_sub_tasks(decoder_handle_t *handle)
{
    // Handle bit decoder task
    switch (handle->bit_decoder)
    {
    case BIT_DECODER_FSK:
        fsk_decoder_task((fsk_decoder_handle_t *)handle->bit_decoder_handle, handle);
        break;
    case BIT_DECODER_NONE:
        // No bit decoder set
        break;
    default:
        LOG_ERROR("Unknown bit decoder type");
        break;
    }
}