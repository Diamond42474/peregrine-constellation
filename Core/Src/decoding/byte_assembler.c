#include "decoding/byte_assembler.h"

#include <stdlib.h>
#include <stdint.h>

#include "c-logger.h"

// Prvate function declarations
static uint16_t swap16(uint16_t v);
static int _process_bit(byte_assembler_handle_t *handle, decoder_handle_t *ctx, bool bit);

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
    handle->preamble = 0xABBA; // Default preamble
    handle->preamble_buffer = 0;
    handle->preamble_found = false;
    handle->state = BYTE_ASSEMBLER_WAITING_FOR_PREAMBLE;

    return 0;
}

int byte_assembler_deinit(byte_assembler_handle_t *handle)
{
    if (!handle)
    {
        LOG_ERROR("Byte assembler handle is NULL");
        return -1;
    }

    return 0;
}

int byte_assembler_set_preamble(byte_assembler_handle_t *handle, uint16_t preamble)
{
    if (!handle)
    {
        LOG_ERROR("Byte assembler handle is NULL");
        return -1;
    }

    handle->preamble = preamble;

    return 0;
}

int byte_assembler_process_bit(byte_assembler_handle_t *handle, decoder_handle_t *ctx, bool bit)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Byte assembler handle is NULL");
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

    if (_process_bit(handle, ctx, bit))
    {
        LOG_ERROR("Failed to process bit");
        ret = -1;
        goto failed;
    }

failed:
    return ret;
}

int byte_assembler_reset(byte_assembler_handle_t *handle)
{
    if (!handle)
    {
        LOG_ERROR("Byte assembler handle is NULL");
        return -1;
    }

    handle->current_byte = 0;
    handle->bits_collected = 0;
    handle->preamble_buffer = 0;
    handle->preamble_found = false;

    return 0;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// PRIVATE FUNCTIONS
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=

static int _process_bit(byte_assembler_handle_t *handle, decoder_handle_t *ctx, bool bit)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Byte assembler handle is NULL");
        return -1;
    }

    switch (handle->state)
    {
    case BYTE_ASSEMBLER_WAITING_FOR_PREAMBLE:

        break;
    case BYTE_ASSEMBLER_ASSEMBLING:
        break;
    default:
        LOG_ERROR("Invalid byte assembler state");
        ret = -1;
        goto failed;
        break;
    }

    handle->preamble_buffer =
        (handle->preamble_buffer << 1) |
        (uint16_t)(bit & 1);

    uint16_t preamble = handle->preamble_buffer;

    LOG_DEBUG("Preamble buffer: 0x%04X", preamble);

    if (preamble == handle->preamble)
    {
        handle->preamble_found = true;
        LOG_INFO("Preamble detected, aligning bytes");
        // Reset byte assembly state
        handle->current_byte = 0;
        handle->bits_collected = 0;

        uint8_t hi = (preamble >> 8) & 0xFF;
        uint8_t lo = preamble & 0xFF;

        if (decoder_process_byte(ctx, hi))
        {
            LOG_ERROR("Failed to process preamble high byte");
            ret = -1;
            goto failed;
        }

        if (decoder_process_byte(ctx, lo))
        {
            LOG_ERROR("Failed to process preamble low byte");
            ret = -1;
            goto failed;
        }

        return 0;
    }

    if (!handle->preamble_found)
    {
        // Still looking for preamble, don't assemble bytes yet
        return 0;
    }

    handle->current_byte |= (bit & 0x01) << (7 - handle->bits_collected);

    handle->bits_collected++;

    if (handle->bits_collected >= 8)
    {
        LOG_DEBUG("Assembled byte: 0x%02X", handle->current_byte);
        if (decoder_process_byte(ctx, handle->current_byte))
        {
            LOG_ERROR("Failed to process assembled byte");
            ret = -1;
            goto failed;
        }
        handle->current_byte = 0;
        handle->bits_collected = 0;
    }

failed:
    return ret;
}

static uint16_t swap16(uint16_t v)
{
    return (v >> 8) | (v << 8);
}