#include "unity.h"

#include "decoding/fsk_decoder.h"
#include "c-logger.h"

#define F0 (1100.0f)
#define F1 (2200.0f)
#define POWER_THRESHOLD (1E10)
#define SAMPLE_RATE (26400)      // Based off decoder_example.c calculated for 32 baud
#define SYMBOL_SAMPLE_SIZE (825) // Based off decoder_example.c calculated for 32 baud
#define BUFFER_SYMBOL_COUNT (4)  // Minimum of 3 is good for timing recovery

static fsk_decoder_handle_t handle;

void setUp(void)
{
}

void tearDown(void)
{
}

void test_init(void)
{
    TEST_ASSERT_TRUE(fsk_decoder_init(&handle) == 0);
    TEST_ASSERT_TRUE(fsk_decoder_set_frequencies(&handle, F0, F1) == 0);
    TEST_ASSERT_TRUE(fsk_decoder_set_power_threshold(&handle, POWER_THRESHOLD) == 0);
    TEST_ASSERT_TRUE(fsk_decoder_set_sample_rate(&handle, SAMPLE_RATE) == 0);
    TEST_ASSERT_TRUE(fsk_decoder_set_symbol_sample_size(&handle, SYMBOL_SAMPLE_SIZE, BUFFER_SYMBOL_COUNT) == 0);
}

int main(void)
{
    UNITY_BEGIN();

    log_init(LOG_LEVEL_INFO);

    RUN_TEST(test_init);

    return UNITY_END();
}