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
    byte_assembler_reset(&byte_assembler_handle);
}

static void _send_byte_as_bits(byte_assembler_handle_t *handle, decoder_handle_t *ctx, unsigned char byte, bool msb_first)
{
    for (int i = 0; i < 8; i++)
    {
        bool bit = (byte >> (7 - i)) & 0x01; // MSB first
        if (!msb_first)
        {
            bit = (byte >> i) & 0x01; // LSB first
        }
        byte_assembler_process_bit(handle, ctx, bit);
    }
}

void test_preamble_detection(void)
{
    TEST_ASSERT_TRUE(byte_assembler_init(&byte_assembler_handle) == 0);

    byte_assembler_set_bit_order(&byte_assembler_handle, BIT_ORDER_MSB_FIRST);
    byte_assembler_set_preamble(&byte_assembler_handle, 0xABBA);

    _send_byte_as_bits(&byte_assembler_handle, &decoder_handle, 0xAB, 1);
    _send_byte_as_bits(&byte_assembler_handle, &decoder_handle, 0xBA, 1);

    TEST_ASSERT_TRUE(byte_assembler_handle.preamble_found);
    TEST_ASSERT_EQUAL_HEX16(0xABBA, byte_assembler_handle.preamble_buffer);
}

void test_preamble_correction(void)
{
    TEST_ASSERT_TRUE(byte_assembler_init(&byte_assembler_handle) == 0);

    byte_assembler_set_bit_order(&byte_assembler_handle, BIT_ORDER_MSB_FIRST);
    byte_assembler_set_preamble(&byte_assembler_handle, 0xABBA);

    // Random bits to offset any bit alignment
    for (int i = 0; i < 13; i++)
    {
        byte_assembler_process_bit(&byte_assembler_handle, &decoder_handle, 1);
    }

    TEST_ASSERT_FALSE(byte_assembler_handle.preamble_found);

    _send_byte_as_bits(&byte_assembler_handle, &decoder_handle, 0xAB, 1);
    _send_byte_as_bits(&byte_assembler_handle, &decoder_handle, 0xBA, 1);

    TEST_ASSERT_TRUE(byte_assembler_handle.preamble_found);

    _send_byte_as_bits(&byte_assembler_handle, &decoder_handle, 0xCD, 1);

    // Check for bit alignment
    TEST_ASSERT_EQUAL_HEX8(0xCD, last_processed_byte);
}

void test_byte_assembler_reset(void)
{
    TEST_ASSERT_TRUE(byte_assembler_init(&byte_assembler_handle) == 0);

    byte_assembler_set_bit_order(&byte_assembler_handle, BIT_ORDER_MSB_FIRST);
    byte_assembler_set_preamble(&byte_assembler_handle, 0xABBA);

    // First detect preamble to set some internal state
    _send_byte_as_bits(&byte_assembler_handle, &decoder_handle, 0xAB, 1);
    _send_byte_as_bits(&byte_assembler_handle, &decoder_handle, 0xBA, 1);

    TEST_ASSERT_TRUE(byte_assembler_handle.preamble_found);

    byte_assembler_reset(&byte_assembler_handle);

    TEST_ASSERT_FALSE(byte_assembler_handle.preamble_found);
    TEST_ASSERT_EQUAL_HEX16(0, byte_assembler_handle.preamble_buffer);
    TEST_ASSERT_EQUAL_HEX8(0, byte_assembler_handle.current_byte);
    TEST_ASSERT_EQUAL_INT(0, byte_assembler_handle.bits_collected);
}

void test_lsb_first_preamble_detection(void)
{
    TEST_ASSERT_TRUE(byte_assembler_init(&byte_assembler_handle) == 0);

    byte_assembler_set_bit_order(&byte_assembler_handle, BIT_ORDER_LSB_FIRST);
    byte_assembler_set_preamble(&byte_assembler_handle, 0xABBA);

    _send_byte_as_bits(&byte_assembler_handle, &decoder_handle, 0xBA, 0);
    _send_byte_as_bits(&byte_assembler_handle, &decoder_handle, 0xAB, 0);

    TEST_ASSERT_TRUE(byte_assembler_handle.preamble_found);
    TEST_ASSERT_EQUAL_HEX16(0xABBA, byte_assembler_handle.preamble_buffer);
}

void test_msb_first_preamble_detection(void)
{
    TEST_ASSERT_TRUE(byte_assembler_init(&byte_assembler_handle) == 0);

    byte_assembler_set_bit_order(&byte_assembler_handle, BIT_ORDER_MSB_FIRST);
    byte_assembler_set_preamble(&byte_assembler_handle, 0xABBA);

    LOG_ERROR("Sending over thing");
    _send_byte_as_bits(&byte_assembler_handle, &decoder_handle, 0xAB, 1);
    _send_byte_as_bits(&byte_assembler_handle, &decoder_handle, 0xBA, 1);

    TEST_ASSERT_TRUE(byte_assembler_handle.preamble_found);
    TEST_ASSERT_EQUAL_HEX16(0xABBA, byte_assembler_handle.preamble_buffer);
}

int main(void)
{
    UNITY_BEGIN();

    log_init(LOG_LEVEL_INFO);

    RUN_TEST(test_preamble_detection);
    RUN_TEST(test_preamble_correction);
    RUN_TEST(test_byte_assembler_reset);
    RUN_TEST(test_lsb_first_preamble_detection);
    RUN_TEST(test_msb_first_preamble_detection);

    return UNITY_END();
}