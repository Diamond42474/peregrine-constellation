#include "unity.h"

#include "decoding/byte_assembler.h"
#include "c-logger.h"
#include <stdbool.h>

extern void mock_decoder_reset(void);
extern void mock_decoder_set_byte_processor(void (*processor)(unsigned char));
extern void mock_decoder_set_bit_processor(void (*processor)(bool));
extern void mock_decoder_set_packet_processor(void (*processor)(packet_t *));

static byte_assembler_handle_t byte_assembler_handle;
static decoder_handle_t decoder_handle;
static unsigned char last_processed_byte = 0;

static void _test_byte_processor(unsigned char byte)
{
    last_processed_byte = byte;
}

void setUp(void)
{
    mock_decoder_reset();
    mock_decoder_set_byte_processor(_test_byte_processor);
}

void tearDown(void)
{
    // This is run after EACH TEST
}

static void _send_byte_as_bits(byte_assembler_handle_t *handle, decoder_handle_t *ctx, unsigned char byte)
{
    for (int i = 0; i < 8; i++)
    {
        bool bit = (byte >> (7 - i)) & 0x01; // MSB first
        byte_assembler_process_bit(handle, ctx, bit);
    }
}

void test_preamble_detection(void)
{
    TEST_ASSERT_TRUE(byte_assembler_init(&byte_assembler_handle) == 0);

    byte_assembler_set_preamble(&byte_assembler_handle, 0xABBA);

    _send_byte_as_bits(&byte_assembler_handle, &decoder_handle, 0xAB);

    TEST_ASSERT_EQUAL_HEX8(0xAB, last_processed_byte);

    _send_byte_as_bits(&byte_assembler_handle, &decoder_handle, 0xAB);
    _send_byte_as_bits(&byte_assembler_handle, &decoder_handle, 0xBA);

    for (int i = 0; i < 6; i++)
    {
        byte_assembler_process_bit(&byte_assembler_handle, &decoder_handle, 1);
    }

    _send_byte_as_bits(&byte_assembler_handle, &decoder_handle, 0xAB);
    _send_byte_as_bits(&byte_assembler_handle, &decoder_handle, 0xBA);

    _send_byte_as_bits(&byte_assembler_handle, &decoder_handle, 0xCD);

    TEST_ASSERT_EQUAL_HEX8(0xCD, last_processed_byte);
}

int main(void)
{
    UNITY_BEGIN();

    log_init(LOG_LEVEL_INFO);

    RUN_TEST(test_preamble_detection);

    return UNITY_END();
}