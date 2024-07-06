#include "../include/packet.h"
#include <string.h>

uint16_t calculate_checksum(const mesh_packet_t *packet)
{
    uint16_t checksum = 0;
    for (size_t i = 0; i < packet->payload_length; i++)
    {
        checksum += packet->payload[i];
    }
    return checksum;
}

uint16_t calculate_size(const mesh_packet_t *packet)
{
    uint16_t size = 0;
    size += sizeof(packet->packet_type);
    size += sizeof(packet->src_addr);
    size += sizeof(packet->dest_addr);
    size += sizeof(packet->seq_num);
    size += sizeof(packet->payload_length);
    size += sizeof(packet->hop_count);
    size += sizeof(packet->max_hops);
    size += sizeof(packet->checksum);
    size += packet->payload_length;
    return size;
}

uint16_t calculate_min_size()
{
    mesh_packet_t packet;
    uint16_t size = 0;
    size += sizeof(packet.packet_type);
    size += sizeof(packet.src_addr);
    size += sizeof(packet.dest_addr);
    size += sizeof(packet.seq_num);
    size += sizeof(packet.payload_length);
    size += sizeof(packet.hop_count);
    size += sizeof(packet.max_hops);
    size += sizeof(packet.checksum);
    return size;
}

void initialize_packet(mesh_packet_t *packet, uint8_t packet_type, uint16_t src_addr, uint16_t dest_addr, uint16_t seq_num, uint8_t max_hops, const uint8_t *payload, size_t payload_length) {
    // Initialize packet fields
    packet->packet_type = packet_type;
    packet->src_addr = src_addr;
    packet->dest_addr = dest_addr;
    packet->seq_num = seq_num;
    packet->hop_count = 0; // Start with zero hops
    packet->max_hops = max_hops;

    // Limit payload length if it exceeds maximum size
    if (payload_length > pconfigMAX_PAYLOAD_SIZE) {
        payload_length = pconfigMAX_PAYLOAD_SIZE;
    }

    packet->payload_length = payload_length;
    
    // Copy payload into packet
    memcpy(packet->payload, payload, payload_length);
    
    // Calculate checksum for the packet
    packet->checksum = calculate_checksum(packet); // Ensure calculate_checksum() is defined correctly
}


void print_packet(const mesh_packet_t *packet)
{
    printf("Packet type: %d\n", packet->packet_type);
    printf("Source address: 0x%04X\n", packet->src_addr);
    printf("Destination address: 0x%04X\n", packet->dest_addr);
    printf("Sequence number: %d\n", packet->seq_num);
    printf("Payload length: %d\n", packet->payload_length);
    printf("Hop count: %d\n", packet->hop_count);
    printf("Max hops: %d\n", packet->max_hops);
    printf("Checksum: 0x%04X\n", packet->checksum);
    printf("Payload: ");
    for (size_t i = 0; i < packet->payload_length; i++)
    {
        printf("%02X ", packet->payload[i]);
    }
    printf("\n");
}
