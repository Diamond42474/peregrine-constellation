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
    handle->state = PACKET_DECODER_STATE_WAITING_FOR_PREAMBLE;

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

    LOG_INFO("Processing byte: 0x%02X", byte);
    handle->packet_buffer[handle->packet_buffer_index++] = byte;

    switch (handle->state)
    {
    case PACKET_DECODER_STATE_WAITING_FOR_PREAMBLE:
        // Reset if preamble detected
        // TODO: Implement preamble fetch from ctx
        if (handle->packet_buffer_index >= 2)
        {
            // Check for preamble pattern (0xAA, 0xAA)
            if (handle->packet_buffer[handle->packet_buffer_index - 2] == 0xAB &&
                handle->packet_buffer[handle->packet_buffer_index - 1] == 0xBA)
            {
                LOG_INFO("Preamble detected");
                handle->state = PACKET_DECODER_STATE_WAITING_FOR_HEADER;
            }
            packet_decoder_reset(handle);
        }
        break;
    case PACKET_DECODER_STATE_WAITING_FOR_HEADER:

        // Check if we have header (13 bytes)
        if (handle->packet_buffer_index == packet_get_header_size())
        {
            // Extract header fields
            _process_header(handle);

            // Validate header CRC
            uint16_t header_crc = calculate_header_crc(&handle->current_packet);
            if (header_crc != handle->current_packet.content.header_crc)
            {
                LOG_WARN("Header CRC mismatch, resetting packet decoder");
                packet_decoder_reset(handle);
                handle->state = PACKET_DECODER_STATE_WAITING_FOR_PREAMBLE;
                return 0;
            }
            LOG_INFO("Header received and validated, waiting for payload");
            handle->state = PACKET_DECODER_STATE_WAITING_FOR_PAYLOAD;
        }
        break;
    case PACKET_DECODER_STATE_WAITING_FOR_PAYLOAD:
        // Check if we have received the full payload
        if (handle->packet_buffer_index >= packet_get_header_size() + handle->current_packet.content.payload_length)
        {
            // Copy payload data
            for (size_t i = 0; i < handle->current_packet.content.payload_length; i++)
            {
                handle->current_packet.content.payload[i] = handle->packet_buffer[packet_get_header_size() + i];
            }

            // Packet fully received
            LOG_INFO("Full packet received");
            decoder_process_packet(handle->ctx, &handle->current_packet);
            packet_decoder_reset(handle);
            handle->state = PACKET_DECODER_STATE_WAITING_FOR_PREAMBLE;
        }
        break;
    default:
        LOG_ERROR("Invalid packet decoder state");
        handle->state = PACKET_DECODER_STATE_WAITING_FOR_PREAMBLE;
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
    handle->current_packet.content.packet_type = handle->packet_buffer[0];
    handle->current_packet.content.src_addr = (handle->packet_buffer[1] << 8) | handle->packet_buffer[2];
    handle->current_packet.content.dest_addr = (handle->packet_buffer[3] << 8) | handle->packet_buffer[4];
    handle->current_packet.content.seq_num = (handle->packet_buffer[5] << 8) | handle->packet_buffer[6];
    handle->current_packet.content.payload_length = (handle->packet_buffer[7] << 8) | handle->packet_buffer[8];
    handle->current_packet.content.hop_count = handle->packet_buffer[9];
    handle->current_packet.content.max_hops = handle->packet_buffer[10];
    handle->current_packet.content.header_crc = (handle->packet_buffer[11] << 8) | handle->packet_buffer[12];
}