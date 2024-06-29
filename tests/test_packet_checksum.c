#include <assert.h>
#include <stdio.h>
#include "packet.h"

int main() {
    mesh_packet_t packet;
    uint8_t payload[] = {1, 2, 3, 4};

    initialize_packet(&packet, 1, 0x1234, 0x5678, 1, 5, payload, sizeof(payload));

    uint16_t expected_checksum = calculate_checksum(&packet);

    assert(packet.checksum == expected_checksum);
    return 0;
}