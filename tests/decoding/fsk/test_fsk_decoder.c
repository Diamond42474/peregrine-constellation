#include "unity.h"

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "decoding/fsk_decoder.h"
#include "c-logger.h"

#define MAX_LOOPS 1000000 // Safety to prevent infinite loops in tests

#define F0 (1100.0f)
#define F1 (2200.0f)
#define POWER_THRESHOLD (1E10)
#define SAMPLE_RATE (26400)      // Based off decoder_example.c calculated for 32 baud
#define SYMBOL_SAMPLE_SIZE (825) // Based off decoder_example.c calculated for 32 baud
#define BUFFER_SYMBOL_COUNT (3)  // Minimum of 3 is good for timing recovery

extern void mock_decoder_reset(void);
extern void mock_decoder_set_bit_processor(void (*processor)(bool));

static decoder_handle_t decoder_handle;
static fsk_decoder_handle_t handle;
static uint16_t sample_buffer[SYMBOL_SAMPLE_SIZE * BUFFER_SYMBOL_COUNT * 2];
static bool bit_buffer[1024]; // Buffer for decoded bits
static circular_buffer_t bit_circular_buffer;

void bit_cb(bool bit)
{
    LOG_DEBUG("Decoded bit: %d", bit);
    circular_buffer_push(&bit_circular_buffer, &bit);
}

void generate_sine_wave(uint16_t *buffer, float frequency, float sample_rate, uint32_t sample_count)
{
    const float amplitude = 32767.0f; // Half of 16-bit range
    const float offset = 32768.0f;    // Center value for unsigned 16-bit

    for (uint32_t i = 0; i < sample_count; i++)
    {
        float t = (float)i / sample_rate; // Time in seconds
        float value = sinf(2.0f * M_PI * frequency * t);
        buffer[i] = (uint16_t)(offset + amplitude * value);
    }
}

void generate_noise(uint16_t *buffer, float sample_rate, uint32_t sample_count)
{
    for (uint32_t i = 0; i < sample_count; i++)
    {
        buffer[i] = rand() % (32768); // Random value between 0 and 32767
    }
}

void send_bit(bool bit)
{
    uint16_t buffer[SYMBOL_SAMPLE_SIZE];
    if (bit)
    {
        generate_sine_wave(buffer, F1, SAMPLE_RATE, SYMBOL_SAMPLE_SIZE);
    }
    else
    {
        generate_sine_wave(buffer, F0, SAMPLE_RATE, SYMBOL_SAMPLE_SIZE);
    }
    for (int i = 0; i < SYMBOL_SAMPLE_SIZE; i++)
    {
        circular_buffer_push(&decoder_handle.input_buffer, &buffer[i]);
    }
}

void send_noise(int sample_count)
{
    uint16_t buffer[sample_count];
    generate_noise(buffer, SAMPLE_RATE, sample_count);
    for (int i = 0; i < sample_count; i++)
    {
        circular_buffer_push(&decoder_handle.input_buffer, &buffer[i]);
    }
}

void process(void)
{
    for (int i = 0; i < 100; i++)
    {
        fsk_decoder_task(&handle, &decoder_handle);
    }
    return;
    for (int i = 0; i < MAX_LOOPS; i++)
    {
        if (!fsk_decoder_busy(&handle, &decoder_handle))
        {
            break;
        }
        fsk_decoder_task(&handle, &decoder_handle);

        if (i == MAX_LOOPS - 1)
        {
            TEST_FAIL_MESSAGE("FSK decoder did not finish processing in expected time");
        }
    }
}

void setUp(void)
{
    memset(&decoder_handle, 0, sizeof(decoder_handle));
    memset(&handle, 0, sizeof(handle));
    memset(sample_buffer, 0, sizeof(sample_buffer));
    circular_buffer_static_init(&decoder_handle.input_buffer, sample_buffer, sizeof(uint16_t), BUFFER_SYMBOL_COUNT * SYMBOL_SAMPLE_SIZE * 2);
    circular_buffer_static_init(&bit_circular_buffer, bit_buffer, sizeof(bool), sizeof(bit_buffer) / sizeof(bool));
    mock_decoder_set_bit_processor(bit_cb);
}

void tearDown(void)
{
    mock_decoder_reset();
}

void init(void)
{
    TEST_ASSERT_TRUE(fsk_decoder_init(&handle) == 0);
    TEST_ASSERT_TRUE(fsk_decoder_set_frequencies(&handle, F0, F1) == 0);
    TEST_ASSERT_TRUE(fsk_decoder_set_power_threshold(&handle, POWER_THRESHOLD) == 0);
    TEST_ASSERT_TRUE(fsk_decoder_set_sample_rate(&handle, SAMPLE_RATE) == 0);
    TEST_ASSERT_TRUE(fsk_decoder_set_symbol_sample_size(&handle, SYMBOL_SAMPLE_SIZE, BUFFER_SYMBOL_COUNT) == 0);
    process(); // Make sure it initializes everything
}

void simple_bit_decoding(void)
{
    init();

    // fsk decoder requires 3x symbol size to reliably detect signal and recover timing,
    // so we need to send multiple bits to get it into the decoding state

    send_bit(0); // The bit we're looking for
    send_bit(1);
    send_bit(1);
    process();
    bool bit = 1; // Opposite of what it should be to ensure it gets updated
    TEST_ASSERT_TRUE_MESSAGE(circular_buffer_pop(&bit_circular_buffer, &bit) == 0, "Failed to pop bit from circular buffer");
    TEST_ASSERT_EQUAL(bit, false);
}

void detect_signal(void)
{
    init();

    // Send 5 symbols worth of noise
    for (int i = 0; i < 5; i++)
    {
        send_noise(SYMBOL_SAMPLE_SIZE);
        process();
    }

    bool bit = 1;
    TEST_ASSERT_TRUE_MESSAGE(circular_buffer_pop(&bit_circular_buffer, &bit) != 0, "Unexpectedly decoded a bit from noise");

    send_bit(0); // Send a valid bit to ensure it can still decode after noise
    process();

    bit = 1;
    TEST_ASSERT_TRUE_MESSAGE(circular_buffer_pop(&bit_circular_buffer, &bit) == 0, "Failed to pop bit from circular buffer after noise");
    TEST_ASSERT_EQUAL(bit, false);
}

int main(void)
{
    UNITY_BEGIN();

    log_init(LOG_LEVEL_DEBUG);

    RUN_TEST(init);
    RUN_TEST(simple_bit_decoding);
    RUN_TEST(detect_signal);

    return UNITY_END();
}