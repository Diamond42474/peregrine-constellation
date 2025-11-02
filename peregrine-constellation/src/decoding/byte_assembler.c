#include "byte_assembler.h"

#include <stdlib.h>
#include <stdint.h>

#include "c-logger.h"
#include "cobs_decoder.h"

// Prvate function declarations
static int _process_bit(byte_assembler_handle_t *handle, bool bit);

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// PUBLIC FUNCTIONS
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=

int byte_assembler_init(byte_assembler_handle_t *handle)
{
    if (!handle)
    {
        LOG_ERROR("Byte assembler handle is NULL");
        return -1;
    }

    handle->current_byte = 0;
    handle->bits_collected = 0;
    handle->bit_order = BIT_ORDER_MSB_FIRST;
    handle->preamble_byte = 0xAA; // Default preamble
    handle->state = BYTE_ASSEMBLER_STATE_INITIALIZING;

    return 0;
}

int byte_assembler_deinit(byte_assembler_handle_t *handle)
{
    if (!handle)
    {
        LOG_ERROR("Byte assembler handle is NULL");
        return -1;
    }

    if (circular_buffer_deinit(&handle->byte_buffer) != 0)
    {
        LOG_ERROR("Failed to deinitialize byte buffer");
        return -1;
    }

    handle->state = BYTE_ASSEMBLER_STATE_UNINITIALIZED;

    return 0;
}

int byte_assembler_set_bit_order(byte_assembler_handle_t *handle, bit_order_t order)
{
    if (!handle)
    {
        LOG_ERROR("Byte assembler handle is NULL");
        return -1;
    }

    handle->bit_order = order;

    return 0;
}

int byte_assembler_set_byte_buffer_size(byte_assembler_handle_t *handle, size_t size)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Byte assembler handle is NULL");
        return -1;
    }

    if (size == 0)
    {
        LOG_ERROR("Byte buffer size must be greater than 0");
        return -1;
    }

    handle->byte_buffer_size = size;

failed:
    return ret;
}

int byte_assembler_set_bit_buffer_size(byte_assembler_handle_t *handle, size_t size)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Byte assembler handle is NULL");
        return -1;
    }

    if (size == 0)
    {
        LOG_ERROR("Bit buffer size must be greater than 0");
        return -1;
    }

    handle->bit_buffer_size = size;

failed:
    return ret;
}

int byte_assembler_set_preamble(byte_assembler_handle_t *handle, unsigned char preamble)
{
    if (!handle)
    {
        LOG_ERROR("Byte assembler handle is NULL");
        return -1;
    }

    handle->preamble_byte = preamble;

    return 0;
}

int byte_assembler_process_bit(byte_assembler_handle_t *handle, bool bit)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Byte assembler handle is NULL");
        return -1;
    }

    if (handle->state != BYTE_ASSEMBLER_STATE_ASSEMBLING && handle->state != BYTE_ASSEMBLER_STATE_IDLE)
    {
        LOG_WARN("Byte assembler is not in a state to process bits");
        return -1;
    }

    if (bit)
    {
        LOG_DEBUG("Handling 1");
    }
    else
    {
        LOG_DEBUG("Handling 0");
    }
    if (circular_buffer_push(&handle->bit_buffer, &bit) != 0)
    {
        LOG_ERROR("Failed to push bit to buffer");
        ret = -1;
        goto failed;
    }

failed:
    return ret;
}

bool byte_assembler_has_byte(byte_assembler_handle_t *handle)
{
    if (!handle)
    {
        LOG_ERROR("Byte assembler handle is NULL");
        return false;
    }

    return !circular_buffer_is_empty(&handle->byte_buffer);
}

int byte_assembler_get_byte(byte_assembler_handle_t *handle, unsigned char *byte)
{
    int ret = 0;

    if (!handle || !byte)
    {
        LOG_ERROR("Invalid parameters to get byte");
        return -1;
    }

    if (handle->state != BYTE_ASSEMBLER_STATE_ASSEMBLING && handle->state != BYTE_ASSEMBLER_STATE_IDLE)
    {
        LOG_WARN("Byte assembler is not in a state to get bytes");
        return -1;
    }

    if (circular_buffer_is_empty(&handle->byte_buffer))
    {
        LOG_WARN("No bytes available to read");
        return -1; // No bytes available
    }

    if (circular_buffer_pop(&handle->byte_buffer, byte) != 0)
    {
        LOG_ERROR("Failed to pop byte from buffer");
        return -1;
    }

    return ret;
}

int byte_assembler_reset(byte_assembler_handle_t *handle)
{
    if (!handle)
    {
        LOG_ERROR("Byte assembler handle is NULL");
        return -1;
    }

    if (handle->state != BYTE_ASSEMBLER_STATE_ASSEMBLING && handle->state != BYTE_ASSEMBLER_STATE_IDLE)
    {
        LOG_WARN("Byte assembler is not in a state to reset");
        return -1;
    }

    handle->current_byte = 0;
    handle->bits_collected = 0;
    handle->preamble_buffer = 0;
    handle->preamble_bits = 0;
    handle->preamble_found = false;
    handle->state = BYTE_ASSEMBLER_STATE_IDLE;
    circular_buffer_reset(&handle->byte_buffer);
    circular_buffer_reset(&handle->bit_buffer);

    return 0;
}

int byte_assembler_task(byte_assembler_handle_t *handle)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Byte assembler handle is NULL");
        return -1;
    }

    switch (handle->state)
    {
    case BYTE_ASSEMBLER_STATE_INITIALIZING:
        // Initialize circular buffers
        if (circular_buffer_dynamic_init(&handle->bit_buffer, sizeof(bool), handle->bit_buffer_size) != 0)
        {
            LOG_ERROR("Failed to initialize bit buffer");
            ret = -1;
            goto failed;
        }
        if (circular_buffer_dynamic_init(&handle->byte_buffer, sizeof(unsigned char), handle->byte_buffer_size) != 0)
        {
            LOG_ERROR("Failed to initialize byte buffer");
            ret = -1;
            goto failed;
        }
        handle->state = BYTE_ASSEMBLER_STATE_IDLE;
        LOG_INFO("Byte assembler initialized");
        break;
    case BYTE_ASSEMBLER_STATE_IDLE:
        if (!circular_buffer_is_empty(&handle->bit_buffer))
        {
            handle->state = BYTE_ASSEMBLER_STATE_ASSEMBLING;
        }
        break;
    case BYTE_ASSEMBLER_STATE_ASSEMBLING:
        while (!circular_buffer_is_empty(&handle->bit_buffer))
        {
            bool bit;
            if (circular_buffer_pop(&handle->bit_buffer, &bit) != 0)
            {
                LOG_ERROR("Failed to pop bit from buffer");
                ret = -1;
                goto failed;
            }

            if (_process_bit(handle, bit))
            {
                LOG_ERROR("Failed to process bit");
                ret = -1;
                goto failed;
            }
        }
        if (circular_buffer_is_empty(&handle->bit_buffer))
        {
            handle->state = BYTE_ASSEMBLER_STATE_IDLE;
        }
        break;
    case BYTE_ASSEMBLER_STATE_UNINITIALIZED:
        LOG_ERROR("Byte assembler is uninitialized");
        ret = -1;
        break;
    default:
        LOG_ERROR("Unknown byte assembler state");
        handle->state = BYTE_ASSEMBLER_STATE_UNINITIALIZED;
        ret = -1;
        break;
    }
failed:
    return ret;
}

bool byte_assembler_busy(byte_assembler_handle_t *handle)
{
    if (!handle)
    {
        LOG_ERROR("Byte assembler handle is NULL");
        return false;
    }

    return handle->state != BYTE_ASSEMBLER_STATE_IDLE;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// PRIVATE FUNCTIONS
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=

static int _process_bit(byte_assembler_handle_t *handle, bool bit)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Byte assembler handle is NULL");
        return -1;
    }

    if (bit != 0 && bit != 1)
    {
        return 0;
    } // ignore invalid bits

    // Shift bit into preamble buffer
    handle->preamble_buffer = (handle->preamble_buffer << 1) | (bit & 0x01);
    handle->preamble_bits = (handle->preamble_bits < 8) ? handle->preamble_bits + 1 : 8;

    if (handle->preamble_buffer == handle->preamble_byte)
    {
        handle->preamble_found = true;
        LOG_INFO("Preamble detected, aligning bytes");
        // Reset byte assembly state
        handle->current_byte = 0;
        handle->bits_collected = 0;

        // TODO: reset cobs decoder state as well
        return 0;
    }

    // Now assemble bytes as normal
    if (handle->bit_order == BIT_ORDER_LSB_FIRST)
    {
        handle->current_byte |= (bit & 0x01) << handle->bits_collected;
    }
    else // MSB first
    {
        handle->current_byte |= (bit & 0x01) << (7 - handle->bits_collected);
    }

    handle->bits_collected++;

    if (handle->bits_collected >= 8)
    {
        if (circular_buffer_push(&handle->byte_buffer, &handle->current_byte) != 0)
        {
            LOG_ERROR("Failed to push byte to buffer");
            ret = -1;
            goto failed;
        }
        handle->current_byte = 0;
        handle->bits_collected = 0;
    }

failed:
    return ret;
}