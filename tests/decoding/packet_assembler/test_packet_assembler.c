#include "unity.h"

#include "decoding/packet_decoder.h"
#include "encoding/packet_serializer.h"
#include "decoding/decoder.h"
#include "c-logger.h"
#include "packet.h"
#include <stdbool.h>

extern void mock_decoder_reset(void);
extern void mock_decoder_set_byte_processor(void (*processor)(unsigned char));
extern void mock_decoder_set_bit_processor(void (*processor)(bool));
extern void mock_decoder_set_packet_processor(void (*processor)(packet_t *));

static packet_t last_processed_packet;
static decoder_handle_t decoder_handle;
static packet_decoder_t packet_decoder_handle;
static int packets_received = 0;

static void _test_packet_processor(packet_t *packet)
{
    last_processed_packet = *packet;
    print_packet(packet);
    packets_received++;
}

void setUp(void)
{
    mock_decoder_reset();
    mock_decoder_set_packet_processor(_test_packet_processor);
    packet_decoder_init(&packet_decoder_handle, &decoder_handle);
    decoder_init(&decoder_handle);
    packets_received = 0;
}

void tearDown(void)
{
    // No specific teardown needed for now
}

void test_packet_decoding(void)
{
    uint8_t serialized_packet[256];
    circular_buffer_t serialized_buffer;
    circular_buffer_static_init(&serialized_buffer, serialized_packet, sizeof(uint8_t), sizeof(serialized_packet));
    packet_t test_packet = {
        .content = {
            .src_addr = 0x01,
            .dest_addr = 0x02,
            .id = 0x10,
            .ttl = 5,
            .type = 0x01,
            .payload_length = 4,
            .crc = 0x1234,
            .payload = {0xDE, 0xAD, 0xBE, 0xEF}}};
    
    test_packet.content.crc = calculate_crc(&test_packet);

    if (packet_serializer_serialize(&test_packet, &serialized_buffer))
    {
        TEST_FAIL_MESSAGE("Failed to serialize test packet");
    }

    // Simulate feeding serialized bytes into the decoder
    while (circular_buffer_count(&serialized_buffer))
    {
        uint8_t byte;
        circular_buffer_pop(&serialized_buffer, &byte);
        
        packet_decoder_process_byte(&packet_decoder_handle, byte);
    }

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, packets_received, "We should have decoded 1 packet");
}

int main(void)
{
    UNITY_BEGIN();

    log_init(LOG_LEVEL_INFO);

    RUN_TEST(test_packet_decoding);

    return UNITY_END();
}