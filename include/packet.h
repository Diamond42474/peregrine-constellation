#ifndef PACKET_H
#define PACKET_H

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#include "pconfig.h"

typedef struct {
    uint8_t packet_type;
    uint16_t src_addr;
    uint16_t dest_addr;
    uint16_t seq_num;
    uint16_t payload_length;
    uint8_t hop_count;
    uint8_t max_hops;
    uint16_t checksum;
    uint8_t payload[pconfigMAX_PAYLOAD_SIZE];
} mesh_packet_t;

uint16_t calculate_checksum(const mesh_packet_t *packet);
uint16_t calculate_size(const mesh_packet_t *packet);
uint16_t calculate_min_size();
void initialize_packet(mesh_packet_t *packet, uint8_t packet_type, uint16_t src_addr, uint16_t dest_addr, uint16_t seq_num, uint8_t max_hops, const uint8_t *payload, size_t payload_length);
void print_packet(const mesh_packet_t *packet);
#endif // PACKET_H