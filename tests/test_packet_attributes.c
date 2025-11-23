#include <assert.h>
#include <stdio.h>
#include "packet.h"

static void test_payload_length() {
    packet_t packet;
    uint8_t payload[] = {1, 2, 3, 4};
    initialize_packet(&packet, 1, 0x1234, 0x5678, 1, 5, payload, sizeof(payload));
    assert(packet.content.payload_length == sizeof(payload));
    printf("test_payload_length passed\n");
}

static void test_hop_count() {
    packet_t packet;
    uint8_t payload[] = {1, 2, 3, 4};
    initialize_packet(&packet, 1, 0x1234, 0x5678, 1, 5, payload, sizeof(payload));
    assert(packet.content.hop_count == 0);
    printf("test_hop_count passed\n");
}

static void test_max_hops() {
    packet_t packet;
    uint8_t payload[] = {1, 2, 3, 4};
    initialize_packet(&packet, 1, 0x1234, 0x5678, 1, 5, payload, sizeof(payload));
    assert(packet.content.max_hops == 5);
    printf("test_max_hops passed\n");
}

static void test_payload_content() {
    packet_t packet;
    uint8_t payload[] = {1, 2, 3, 4};
    initialize_packet(&packet, 1, 0x1234, 0x5678, 1, 5, payload, sizeof(payload));
    assert(packet.content.payload[0] == 1);
    assert(packet.content.payload[1] == 2);
    assert(packet.content.payload[2] == 3);
    assert(packet.content.payload[3] == 4);
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
