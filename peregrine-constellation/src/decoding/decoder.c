#include "decoder.h"

#include "c-logger.h"
#include "fsk_decoder.h"
#include "byte_assembler.h"
#include "cobs_decoder.h"

typedef struct
{
    size_t length;
    unsigned char data[256];
} frame_t;

static void _handle_sub_tasks(decoder_handle_t *handle);
static void _handle_sub_task_transfers(decoder_handle_t *handle);
static bool _pending_transfers(decoder_handle_t *handle);
static bool _bit_decoder_pending_transfers(decoder_handle_t *handle);
static bool _byte_decoder_pending_transfers(decoder_handle_t *handle);
static bool _frame_decoder_pending_transfers(decoder_handle_t *handle);
static bool _bit_decoder_busy(decoder_handle_t *handle);
static bool _byte_decoder_busy(decoder_handle_t *handle);
static bool _frame_decoder_busy(decoder_handle_t *handle);

int decoder_init(decoder_handle_t *handle)
{
    if (!handle)
    {
        LOG_ERROR("Decoder handle is NULL");
        return -1;
    }

    handle->bit_decoder = BIT_DECODER_NONE;
    handle->byte_decoder = BYTE_DECODER_NONE;
    handle->frame_decoder = FRAME_DECODER_NONE;

    handle->bit_decoder_handle = NULL;
    handle->byte_decoder_handle = NULL;
    handle->frame_decoder_handle = NULL;

    handle->state = DECODER_STATE_INITIALIZING;

    return 0;
}

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
    handle->frame_decoder = FRAME_DECODER_NONE;

    handle->bit_decoder_handle = NULL;
    handle->byte_decoder_handle = NULL;
    handle->frame_decoder_handle = NULL;

    handle->state = DECODER_STATE_UNINITIALIZED;

    return 0;
}

int decoder_set_frame_decoder(decoder_handle_t *handle, frame_decoder_e type, void *frame_decoder_handle)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Decoder handle is NULL");
        return -1;
    }

    if (!frame_decoder_handle)
    {
        LOG_ERROR("Message decoder handle is NULL");
        ret = -1;
        goto failed;
    }

    handle->frame_decoder = type;
    handle->frame_decoder_handle = frame_decoder_handle;

failed:
    return ret;
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

int decoder_task(decoder_handle_t *handle)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Decoder handle is NULL");
        return -1;
    }

    // Run sub-tasks
    _handle_sub_tasks(handle);

    switch (handle->state)
    {
    case DECODER_STATE_UNINITIALIZED:
        LOG_ERROR("Decoder is uninitialized");
        ret = -1;
        goto failed;
        break;
    case DECODER_STATE_INITIALIZING:
        // Initialize anything if needed
        if (_bit_decoder_busy(handle) || _byte_decoder_busy(handle) || _frame_decoder_busy(handle))
        {
            // Still busy initializing
            break;
        }
        LOG_INFO("Decoder initialized");
        handle->state = DECODER_STATE_IDLE;
        break;
    case DECODER_STATE_IDLE:

        // Check if subtasks need to process
        if (_bit_decoder_busy(handle) || _byte_decoder_busy(handle) || _frame_decoder_busy(handle))
        {
            handle->state = DECODER_STATE_PROCESSING;
        }
        break;
    case DECODER_STATE_PROCESSING:

        // Check if subtasks need data transferred
        if (_pending_transfers(handle))
        {
            handle->state = DECODER_STATE_TRANSFERRING;
        }
    case DECODER_STATE_TRANSFERRING:
        // Process decoding steps
        _handle_sub_task_transfers(handle);
        if (!_pending_transfers(handle))
        {
            // Check if there's more data to process
            if (_bit_decoder_busy(handle) || _byte_decoder_busy(handle) || _frame_decoder_busy(handle))
            {
                handle->state = DECODER_STATE_PROCESSING;
            }
            else
            {
                handle->state = DECODER_STATE_IDLE;
            }
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

    // Process samples using the bit decoder
    switch (handle->bit_decoder)
    {
    case BIT_DECODER_FSK:
        if (fsk_decoder_process((fsk_decoder_handle_t *)handle->bit_decoder_handle, samples, num_samples))
        {
            LOG_ERROR("FSK decoder processing failed");
            ret = -1;
            goto failed;
        }
        break;
    case BIT_DECODER_NONE:
        LOG_ERROR("No bit decoder set");
        ret = -1;
        goto failed;
        break;
    default:
        LOG_ERROR("Unknown bit decoder type");
        ret = -1;
        goto failed;
        break;
    }

failed:
    return ret;
}

bool decoder_has_frame(decoder_handle_t *handle)
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

    return _frame_decoder_pending_transfers(handle);
}

int decoder_get_frame(decoder_handle_t *handle, unsigned char *buffer, size_t buffer_len, size_t *frame_len)
{
    int ret = 0;

    if (!handle || !buffer || buffer_len == 0 || !frame_len)
    {
        LOG_ERROR("Invalid arguments to decoder_get_frame");
        return -1;
    }

    if (handle->state == DECODER_STATE_UNINITIALIZED || handle->state == DECODER_STATE_INITIALIZING)
    {
        LOG_ERROR("Decoder is uninitialized");
        return -1;
    }

    switch (handle->frame_decoder)
    {
    case FRAME_DECODER_COBS:
        if (cobs_decoder_get_message((cobs_decoder_t *)handle->frame_decoder_handle, buffer, buffer_len, frame_len))
        {
            LOG_ERROR("Failed to get frame from COBS decoder");
            ret = -1;
            goto failed;
        }
        break;
    case FRAME_DECODER_NONE:
        LOG_ERROR("No frame decoder set");
        ret = -1;
        goto failed;
        break;
    default:
        LOG_ERROR("Unknown frame decoder type");
        ret = -1;
        goto failed;
        break;
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

    return handle->state != DECODER_STATE_IDLE || _bit_decoder_busy(handle) || _byte_decoder_busy(handle) || _frame_decoder_busy(handle);
}

static void _handle_sub_tasks(decoder_handle_t *handle)
{
    // Handle bit decoder task
    switch (handle->bit_decoder)
    {
    case BIT_DECODER_FSK:
        fsk_decoder_task((fsk_decoder_handle_t *)handle->bit_decoder_handle);
        break;
    case BIT_DECODER_NONE:
        // No bit decoder set
        break;
    default:
        LOG_ERROR("Unknown bit decoder type");
        break;
    }

    // Handle byte decoder task
    switch (handle->byte_decoder)
    {
    case BYTE_DECODER_BIT_STUFFING:
        byte_assembler_task((byte_assembler_handle_t *)handle->byte_decoder_handle);
        break;
    case BYTE_DECODER_NONE:
        // No byte decoder set
        break;
    default:
        LOG_ERROR("Unknown byte decoder type");
        break;
    }

    // Handle frame decoder task
    switch (handle->frame_decoder)
    {
    case FRAME_DECODER_COBS:
        // TODO: COBS decoder task
        cobs_decoder_task((cobs_decoder_t *)handle->frame_decoder_handle);
        break;
    case FRAME_DECODER_NONE:
        // No frame decoder set
        break;
    default:
        LOG_ERROR("Unknown frame decoder type");
        break;
    }
}

static void _handle_sub_task_transfers(decoder_handle_t *handle)
{
    // Check if bit decoder has data
    if (_bit_decoder_pending_transfers(handle))
    {
        bool bit;
        bool received_bit = false;
        switch (handle->bit_decoder)
        {
        case BIT_DECODER_FSK:
            if (fsk_decoder_get_bit((fsk_decoder_handle_t *)handle->bit_decoder_handle, &bit))
            {
                LOG_ERROR("Failed to get bit from FSK decoder");
                received_bit = false;
            }
            else
            {
                received_bit = true;
            }
            break;
        case BIT_DECODER_NONE:
            LOG_ERROR("No bit decoder set");
            return;
            break;
        default:
            LOG_ERROR("Unknown bit decoder type");
            return;
            break;
        }

        if (received_bit)
        {
            // Push bit to byte decoder
            switch (handle->byte_decoder)
            {
            case BYTE_DECODER_BIT_STUFFING:
                if (byte_assembler_process_bit((byte_assembler_handle_t *)handle->byte_decoder_handle, bit))
                {
                    LOG_ERROR("Failed to process bit in byte assembler");
                }
                break;
            case BYTE_DECODER_NONE:
                LOG_ERROR("No byte decoder set");
                return;
                break;
            default:
                LOG_ERROR("Unknown byte decoder type");
                return;
                break;
            }
        }
    }

    // Check if byte decoder has data
    if (_byte_decoder_pending_transfers(handle))
    {
        unsigned char byte;
        bool received_byte = false;
        switch (handle->byte_decoder)
        {
        case BYTE_DECODER_BIT_STUFFING:
            if (byte_assembler_get_byte((byte_assembler_handle_t *)handle->byte_decoder_handle, &byte))
            {
                LOG_ERROR("Failed to get byte from byte assembler");
                received_byte = false;
            }
            else
            {
                received_byte = true;
            }
            break;
        case BYTE_DECODER_NONE:
            LOG_ERROR("No byte decoder set");
            return;
            break;
        default:
            LOG_ERROR("Unknown byte decoder type");
            return;
            break;
        }

        if (received_byte)
        {
            // Push byte to frame decoder
            switch (handle->frame_decoder)
            {
            case FRAME_DECODER_COBS:
                if (cobs_decoder_process((cobs_decoder_t *)handle->frame_decoder_handle, byte))
                {
                    LOG_ERROR("Failed to process byte in COBS decoder");
                }
                break;
            case FRAME_DECODER_NONE:
                LOG_ERROR("No frame decoder set");
                return;
                break;
            default:
                LOG_ERROR("Unknown frame decoder type");
                return;
                break;
            }
        }
    }
}

static bool _pending_transfers(decoder_handle_t *handle)
{
    return _bit_decoder_pending_transfers(handle) || _byte_decoder_pending_transfers(handle) || _frame_decoder_pending_transfers(handle);
}

static bool _bit_decoder_pending_transfers(decoder_handle_t *handle)
{
    switch (handle->bit_decoder)
    {
    case BIT_DECODER_FSK:
        return fsk_decoder_has_bit((fsk_decoder_handle_t *)handle->bit_decoder_handle);
        break;
    case BIT_DECODER_NONE:
        LOG_ERROR("No bit decoder set");
        return false;
        break;
    default:
        LOG_ERROR("Unknown bit decoder type");
        return false;
        break;
    }
}

static bool _byte_decoder_pending_transfers(decoder_handle_t *handle)
{
    switch (handle->byte_decoder)
    {
    case BYTE_DECODER_BIT_STUFFING:
        return byte_assembler_has_byte((byte_assembler_handle_t *)handle->byte_decoder_handle);
        break;
    case BYTE_DECODER_NONE:
        LOG_ERROR("No byte decoder set");
        return false;
        break;
    default:
        LOG_ERROR("Unknown byte decoder type");
        return false;
        break;
    }
}

static bool _frame_decoder_pending_transfers(decoder_handle_t *handle)
{
    switch (handle->frame_decoder)
    {
    case FRAME_DECODER_COBS:
        return cobs_decoder_has_message((cobs_decoder_t *)handle->frame_decoder_handle);
        break;
    case FRAME_DECODER_NONE:
        LOG_ERROR("No frame decoder set");
        return false;
        break;
    default:
        LOG_ERROR("Unknown frame decoder type");
        return false;
        break;
    }
}

static bool _bit_decoder_busy(decoder_handle_t *handle)
{
    switch (handle->bit_decoder)
    {
    case BIT_DECODER_FSK:
        return fsk_decoder_busy((fsk_decoder_handle_t *)handle->bit_decoder_handle);
        break;
    case BIT_DECODER_NONE:
        LOG_ERROR("No bit decoder set");
        return false;
        break;
    default:
        LOG_ERROR("Unknown bit decoder type");
        return false;
        break;
    }
}

static bool _byte_decoder_busy(decoder_handle_t *handle)
{
    switch (handle->byte_decoder)
    {
    case BYTE_DECODER_BIT_STUFFING:
        return byte_assembler_busy((byte_assembler_handle_t *)handle->byte_decoder_handle);
        break;
    case BYTE_DECODER_NONE:
        LOG_ERROR("No byte decoder set");
        return false;
        break;
    default:
        LOG_ERROR("Unknown byte decoder type");
        return false;
        break;
    }
}

static bool _frame_decoder_busy(decoder_handle_t *handle)
{
    switch (handle->frame_decoder)
    {
    case FRAME_DECODER_COBS:
        return cobs_decoder_busy((cobs_decoder_t *)handle->frame_decoder_handle);
        break;
    case FRAME_DECODER_NONE:
        LOG_ERROR("No frame decoder set");
        return false;
        break;
    default:
        LOG_ERROR("Unknown frame decoder type");
        return false;
        break;
    }
}