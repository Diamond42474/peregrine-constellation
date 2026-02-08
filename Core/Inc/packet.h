#ifndef PACKET_H
#define PACKET_H

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#include "pconfig.h"

typedef struct
{
    /**
     * @brief Metadata about the packet (e.g., timestamps, RSSI, etc.)
     */
    struct
    {
        
    } metadata;

    /**
     * @brief Actual sent/received packet content
     */
    struct
    {
        uint8_t packet_type;
        uint16_t src_addr;
        uint16_t dest_addr;
        uint16_t seq_num;
        uint16_t payload_length;
        uint8_t hop_count;
        uint8_t max_hops;
        uint16_t header_crc;
        uint8_t payload[pconfigMAX_PAYLOAD_SIZE];
        uint16_t payload_crc;
    } content;
} packet_t;

uint16_t calculate_payload_crc(const packet_t *packet);
uint16_t calculate_header_crc(const packet_t *packet);
uint16_t calculate_size(const packet_t *packet);
uint16_t calculate_min_size();
size_t packet_get_header_size();
void initialize_packet(packet_t *packet, uint8_t packet_type, uint16_t src_addr, uint16_t dest_addr, uint16_t seq_num, uint8_t max_hops, const uint8_t *payload, size_t payload_length);
void print_packet(const packet_t *packet);
#endif // PACKET_H