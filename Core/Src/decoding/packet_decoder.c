#include "decoding/packet_decoder.h"

#include "decoding/decoder.h"
#include "c-logger.h"
#include <string.h>

static void _process_header(packet_decoder_t *handle);

int packet_decoder_init(packet_decoder_t *handle, void *ctx)
{
    if (!handle || !ctx)
    {
        return -1;
    }

    handle->ctx = ctx;
    handle->state = PACKET_DECODER_STATE_WAITING_FOR_HEADER;

    return 0;
}

int packet_decoder_deinit(packet_decoder_t *handle)
{
    if (!handle)
    {
        return -1;
    }

    // Currently, no dynamic resources to free

    return 0;
}

int packet_decoder_process_byte(packet_decoder_t *handle, unsigned char byte)
{
    if (!handle)
    {
        return -1;
    }

    LOG_INFO("Byte received: 0x%02X\t%c", byte, byte);

    return 0;
    LOG_INFO("Processing byte: 0x%02X", byte);
    handle->packet_buffer[handle->packet_buffer_index++] = byte;

    switch (handle->state)
    {
    case PACKET_DECODER_STATE_WAITING_FOR_HEADER:

        // Check if we have header
        if (handle->packet_buffer_index >= PACKET_HEADER_SIZE)
        {
            // Extract header fields
            _process_header(handle);

            if (handle->current_packet.content.payload_length > pconfigMAX_PAYLOAD_SIZE)
            {
                LOG_ERROR("Invalid payload length: %d", handle->current_packet.content.payload_length);
                packet_decoder_reset(handle);
                handle->state = PACKET_DECODER_STATE_WAITING_FOR_HEADER;
                return -1;
            }

            LOG_INFO("Header received and validated, waiting for payload");
            handle->state = PACKET_DECODER_STATE_WAITING_FOR_PAYLOAD;
        }
        break;
    case PACKET_DECODER_STATE_WAITING_FOR_PAYLOAD:
        // Check if we have received the full payload
        if (handle->packet_buffer_index >= PACKET_HEADER_SIZE + handle->current_packet.content.payload_length)
        {
            // Copy payload data
            for (size_t i = 0; i < handle->current_packet.content.payload_length; i++)
            {
                handle->current_packet.content.payload[i] = handle->packet_buffer[PACKET_HEADER_SIZE + i];
            }

            // Packet fully received
            LOG_INFO("Full packet received");
            decoder_process_packet(handle->ctx, &handle->current_packet);
            packet_decoder_reset(handle);
            handle->state = PACKET_DECODER_STATE_WAITING_FOR_HEADER;
        }
        break;
    default:
        LOG_ERROR("Invalid packet decoder state");
        handle->state = PACKET_DECODER_STATE_WAITING_FOR_HEADER;
        return -1;
    }

    return 0;
}

int packet_decoder_reset(packet_decoder_t *handle)
{
    if (!handle)
    {
        return -1;
    }

    handle->packet_buffer_index = 0;
    // Clear current packet
    memset(&handle->current_packet, 0, sizeof(packet_t));

    return 0;
}

static void _process_header(packet_decoder_t *handle)
{
    if (!handle)
    {
        return;
    }

    // Parse header fields from packet_buffer
    handle->current_packet.content.src_addr = handle->packet_buffer[0];
    handle->current_packet.content.dest_addr = handle->packet_buffer[1];
    handle->current_packet.content.id = handle->packet_buffer[2];
    handle->current_packet.content.ttl = handle->packet_buffer[3] >> 4;
    handle->current_packet.content.type = handle->packet_buffer[3] & 0x0F;
    handle->current_packet.content.payload_length = handle->packet_buffer[4];
    handle->current_packet.content.crc = (handle->packet_buffer[5] << 8) | handle->packet_buffer[6];
}