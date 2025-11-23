#include <assert.h>
#include <stdio.h>
#include "packet.h"

int main() {
    packet_t packet;
    uint8_t payload[] = {1, 2, 3, 4};

    initialize_packet(&packet, 1, 0x1234, 0x5678, 1, 5, payload, sizeof(payload));

    uint16_t payload_crc = calculate_payload_crc(&packet);
    uint16_t header_crc = calculate_header_crc(&packet);

    assert(packet.content.payload_crc == payload_crc);
    assert(packet.content.header_crc == header_crc);
    return 0;
}