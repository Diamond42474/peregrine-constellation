#include "encoding/packet_serializer.h"
#include "c-logger.h"
#include "interface/pconfig.h"
#include <string.h>

int packet_serializer_serialize(const packet_t *packet, circular_buffer_t *output)
{
    if (!packet || !output)
    {
        LOG_ERROR("Packet or output buffer is NULL");
        return -1;
    }

    uint8_t sync_word[2] = {pconfigPREAMBLE_BYTE_1, pconfigPREAMBLE_BYTE_2};
    uint8_t tmp[PACKET_HEADER_SIZE];
    memset(tmp, 0, sizeof(tmp));

    // Push header fields
    tmp[0] = packet->content.src_addr;
    tmp[1] = packet->content.dest_addr;
    tmp[2] = packet->content.id;
    tmp[3] = (packet->content.ttl << 4) | (packet->content.type & 0x0F);
    tmp[4] = packet->content.payload_length;
    tmp[5] = (packet->content.crc >> 8) & 0xFF;
    tmp[6] = packet->content.crc & 0xFF;

    // Push preamble/sync word to output buffer
    if (circular_buffer_push(output, &sync_word[0]))
    {
        LOG_ERROR("Failed to push preamble byte 1");
        return -1;
    }
    if (circular_buffer_push(output, &sync_word[1]))
    {
        LOG_ERROR("Failed to push preamble byte 2");
        return -1;
    }

    // Push packet header to output buffer
    for (size_t i = 0; i < PACKET_HEADER_SIZE; i++)
    {
        if (circular_buffer_push(output, &tmp[i]))
        {
            LOG_ERROR("Failed to push packet byte %zu", i);
            return -1;
        }
    }

    // Push payload to output buffer
    for (size_t i = 0; i < packet->content.payload_length; i++)
    {
        if (circular_buffer_push(output, &packet->content.payload[i]))
        {
            LOG_ERROR("Failed to push payload byte %zu", i);
            return -1;
        }
    }

    return 0;
}