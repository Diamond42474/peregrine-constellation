#include "../Inc/packet.h"
#include <string.h>

uint16_t calculate_payload_crc(const packet_t *packet)
{
    uint16_t header_crc = 0;
    for (size_t i = 0; i < packet->content.payload_length; i++)
    {
        header_crc += packet->content.payload[i];
    }
    return header_crc;
}

uint16_t calculate_header_crc(const packet_t *packet)
{
    uint16_t header_crc = 0;
    header_crc += packet->content.packet_type;
    header_crc += (packet->content.src_addr >> 8) & 0xFF;
    header_crc += packet->content.src_addr & 0xFF;
    header_crc += (packet->content.dest_addr >> 8) & 0xFF;
    header_crc += packet->content.dest_addr & 0xFF;
    header_crc += (packet->content.seq_num >> 8) & 0xFF;
    header_crc += packet->content.seq_num & 0xFF;
    header_crc += (packet->content.payload_length >> 8) & 0xFF;
    header_crc += packet->content.payload_length & 0xFF;
    header_crc += packet->content.hop_count;
    header_crc += packet->content.max_hops;
    return header_crc;
}

uint16_t calculate_size(const packet_t *packet)
{
    uint16_t size = 0;
    size += sizeof(packet->content.packet_type);
    size += sizeof(packet->content.src_addr);
    size += sizeof(packet->content.dest_addr);
    size += sizeof(packet->content.seq_num);
    size += sizeof(packet->content.payload_length);
    size += sizeof(packet->content.hop_count);
    size += sizeof(packet->content.max_hops);
    size += sizeof(packet->content.header_crc);
    size += packet->content.payload_length;
    return size;
}

uint16_t calculate_min_size()
{
    packet_t packet;
    uint16_t size = 0;
    size += sizeof(packet.content.packet_type);
    size += sizeof(packet.content.src_addr);
    size += sizeof(packet.content.dest_addr);
    size += sizeof(packet.content.seq_num);
    size += sizeof(packet.content.payload_length);
    size += sizeof(packet.content.hop_count);
    size += sizeof(packet.content.max_hops);
    size += sizeof(packet.content.header_crc);
    return size;
}

size_t packet_get_header_size()
{
    size_t size = 0;
    size += sizeof(((packet_t *)0)->content.packet_type);
    size += sizeof(((packet_t *)0)->content.src_addr);
    size += sizeof(((packet_t *)0)->content.dest_addr);
    size += sizeof(((packet_t *)0)->content.seq_num);
    size += sizeof(((packet_t *)0)->content.payload_length);
    size += sizeof(((packet_t *)0)->content.hop_count);
    size += sizeof(((packet_t *)0)->content.max_hops);
    size += sizeof(((packet_t *)0)->content.header_crc);
    return size;
}

void initialize_packet(packet_t *packet, uint8_t packet_type, uint16_t src_addr, uint16_t dest_addr, uint16_t seq_num, uint8_t max_hops, const uint8_t *payload, size_t payload_length) {
    // Initialize packet fields
    packet->content.packet_type = packet_type;
    packet->content.src_addr = src_addr;
    packet->content.dest_addr = dest_addr;
    packet->content.seq_num = seq_num;
    packet->content.hop_count = 0; // Start with zero hops
    packet->content.max_hops = max_hops;

    // Limit payload length if it exceeds maximum size
    if (payload_length > pconfigMAX_PAYLOAD_SIZE) {
        payload_length = pconfigMAX_PAYLOAD_SIZE;
    }

    packet->content.payload_length = payload_length;
    
    // Copy payload into packet
    memcpy(packet->content.payload, payload, payload_length);
    
    // Calculate header_crc for the packet
    packet->content.header_crc = calculate_header_crc(packet); // Ensure calculate_header_crc() is defined correctly
}


void print_packet(const packet_t *packet)
{
    printf("Packet type: %d\n", packet->content.packet_type);
    printf("Source address: 0x%04X\n", packet->content.src_addr);
    printf("Destination address: 0x%04X\n", packet->content.dest_addr);
    printf("Sequence number: %d\n", packet->content.seq_num);
    printf("Payload length: %d\n", packet->content.payload_length);
    printf("Hop count: %d\n", packet->content.hop_count);
    printf("Max hops: %d\n", packet->content.max_hops);
    printf("Checksum: 0x%04X\n", packet->content.header_crc);
    printf("Payload: ");
    for (size_t i = 0; i < packet->content.payload_length; i++)
    {
        printf("%02X ", packet->content.payload[i]);
    }
    printf("\n");
}
