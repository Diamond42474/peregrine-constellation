#include "packet.h"
#include <string.h>
#include "c-logger.h"

uint16_t calculate_crc(const packet_t *packet)
{
    uint16_t crc = 0;
    crc += packet->content.src_addr;
    crc += packet->content.dest_addr;
    crc += packet->content.id;
    crc += (packet->content.ttl << 4) | packet->content.type;
    crc += packet->content.payload_length;
    for (size_t i = 0; i < packet->content.payload_length; i++)
    {
        crc += packet->content.payload[i];
    }
    return crc;
}

int initialize_packet(packet_t *packet, packet_type_e packet_type, uint16_t src_addr, uint16_t dest_addr, uint8_t id, const uint8_t *payload, size_t payload_length)
{
    // Initialize packet fields
    packet->content.src_addr = src_addr;
    packet->content.dest_addr = dest_addr;
    packet->content.id = id;
    packet->content.ttl = pconfigTTL; // Default TTL value, can be adjusted as
    packet->content.type = packet_type;

    // Limit payload length if it exceeds maximum size
    if (payload_length > pconfigMAX_PAYLOAD_SIZE)
    {
        LOG_ERROR("Payload larger than max size");
        return -1;
    }

    packet->content.payload_length = payload_length;

    // Copy payload into packet
    memcpy(packet->content.payload, payload, payload_length);

    // Calculate header_crc for the packet
    packet->content.crc = calculate_crc(packet);

    return 0;
}

void print_packet(const packet_t *packet)
{
    printf("Packet type: %d\n", packet->content.type);
    printf("Source address: 0x%02X\n", packet->content.src_addr);
    printf("Destination address: 0x%02X\n", packet->content.dest_addr);
    printf("ID: %d\n", packet->content.id);
    printf("Payload length: %d\n", packet->content.payload_length);
    printf("TTL: %d\n", packet->content.ttl);
    printf("Checksum: 0x%04X\n", packet->content.crc);
    printf("Payload: ");
    for (size_t i = 0; i < packet->content.payload_length; i++)
    {
        printf("%02X ", packet->content.payload[i]);
    }
    printf("\n");
}
