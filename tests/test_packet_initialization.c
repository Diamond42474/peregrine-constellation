#include <assert.h>
#include <stdio.h>
#include "packet.h"

static void test_packet_type() {
    mesh_packet_t packet;
    uint8_t payload[] = {1, 2, 3, 4};
    initialize_packet(&packet, 1, 0x1234, 0x5678, 1, 5, payload, sizeof(payload));
    assert(packet.packet_type == 1);
    printf("test_packet_type passed\n");
}

static void test_src_addr() {
    mesh_packet_t packet;
    uint8_t payload[] = {1, 2, 3, 4};
    initialize_packet(&packet, 1, 0x1234, 0x5678, 1, 5, payload, sizeof(payload));
    assert(packet.src_addr == 0x1234);
    printf("test_src_addr passed\n");
}

static void test_dest_addr() {
    mesh_packet_t packet;
    uint8_t payload[] = {1, 2, 3, 4};
    initialize_packet(&packet, 1, 0x1234, 0x5678, 1, 5, payload, sizeof(payload));
    assert(packet.dest_addr == 0x5678);
    printf("test_dest_addr passed\n");
}

static void test_seq_num() {
    mesh_packet_t packet;
    uint8_t payload[] = {1, 2, 3, 4};
    initialize_packet(&packet, 1, 0x1234, 0x5678, 1, 5, payload, sizeof(payload));
    assert(packet.seq_num == 1);
    printf("test_seq_num passed\n");
}

int main() {
    test_packet_type();
    test_src_addr();
    test_dest_addr();
    test_seq_num();
    printf("All initialization tests passed!\n");
    return 0;
}
