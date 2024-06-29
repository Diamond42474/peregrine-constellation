#include <assert.h>
#include <stdio.h>
#include "packet.h"

static void test_payload_length() {
    mesh_packet_t packet;
    uint8_t payload[] = {1, 2, 3, 4};
    initialize_packet(&packet, 1, 0x1234, 0x5678, 1, 5, payload, sizeof(payload));
    assert(packet.payload_length == sizeof(payload));
    printf("test_payload_length passed\n");
}

static void test_hop_count() {
    mesh_packet_t packet;
    uint8_t payload[] = {1, 2, 3, 4};
    initialize_packet(&packet, 1, 0x1234, 0x5678, 1, 5, payload, sizeof(payload));
    assert(packet.hop_count == 0);
    printf("test_hop_count passed\n");
}

static void test_max_hops() {
    mesh_packet_t packet;
    uint8_t payload[] = {1, 2, 3, 4};
    initialize_packet(&packet, 1, 0x1234, 0x5678, 1, 5, payload, sizeof(payload));
    assert(packet.max_hops == 5);
    printf("test_max_hops passed\n");
}

static void test_payload_content() {
    mesh_packet_t packet;
    uint8_t payload[] = {1, 2, 3, 4};
    initialize_packet(&packet, 1, 0x1234, 0x5678, 1, 5, payload, sizeof(payload));
    assert(packet.payload[0] == 1);
    assert(packet.payload[1] == 2);
    assert(packet.payload[2] == 3);
    assert(packet.payload[3] == 4);
    printf("test_payload_content passed\n");
}

int main() {
    test_payload_length();
    test_hop_count();
    test_max_hops();
    test_payload_content();
    printf("All attribute tests passed!\n");
    return 0;
}
