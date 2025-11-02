#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "decoder.h"
#include "c-logger.h"

#include "fsk_decoder.h"
#include "byte_assembler.h"
#include "cobs_decoder.h"

#include "fsk_utils.h"

#define FQ0 1100.0f
#define FQ1 2200.0f
#define BAUD_RATE 8

#define MIN_ADC_SAMPLE_RATE (FQ1 * 3)
#define MAX_ADC_SAMPLE_RATE (MIN_ADC_SAMPLE_RATE * 2)
#define MIN_SAMPLES_PER_BIT 64
#define MAX_SAMPLES_PER_BIT 1024 * 2

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

int main(void)
{
    int ret = 0;

    log_init(LOG_LEVEL_DEBUG);

    decoder_handle_t decoder;
    fsk_decoder_handle_t fsk_decoder;
    byte_assembler_handle_t byte_assembler;
    cobs_decoder_t cobs_decoder;

    // Calculate FSK parameters
    fsk_timing_t fsk_timing = fsk_calculate_timing(
        FQ1,
        FQ0,
        BAUD_RATE,
        MIN_ADC_SAMPLE_RATE,
        MAX_ADC_SAMPLE_RATE,
        MIN_SAMPLES_PER_BIT,
        MAX_SAMPLES_PER_BIT);
    printf("Recommended Sample Rate: %.2f Hz\n", fsk_timing.sample_rate);
    printf("Samples per Bit: %zu\n", fsk_timing.samples_per_bit);

    // Initialize FSK Decoder
    fsk_decoder_init(&fsk_decoder);
    fsk_decoder_set_rates(&fsk_decoder, (int)fsk_timing.samples_per_bit, (int)fsk_timing.sample_rate);
    fsk_decoder_set_frequencies(&fsk_decoder, FQ0, FQ1);
    fsk_decoder_set_power_threshold(&fsk_decoder, 1000.0f);
    fsk_decoder_set_bit_buffer_size(&fsk_decoder, BAUD_RATE * 10); // Buffer for 10 bits
    fsk_decoder_set_sample_buffer_multiplier(&fsk_decoder, 2);     // 8x sample buffer

    // Initialize Byte Assembler
    byte_assembler_init(&byte_assembler);
    byte_assembler_set_bit_order(&byte_assembler, BIT_ORDER_LSB_FIRST);
    byte_assembler_set_preamble(&byte_assembler, 0xAA);                  // Preamble byte
    byte_assembler_set_bit_buffer_size(&byte_assembler, BAUD_RATE * 10); // Buffer for 10 bits
    byte_assembler_set_byte_buffer_size(&byte_assembler, 256);           // Buffer for 256 bytes

    // Initialize COBS Decoder
    cobs_decoder_init(&cobs_decoder);
    cobs_decoder_set_input_buffer_size(&cobs_decoder, 512);
    cobs_decoder_set_output_buffer_size(&cobs_decoder, 128);

    // Initialize Decoder
    decoder_init(&decoder);
    decoder_set_bit_decoder(&decoder, BIT_DECODER_FSK, &fsk_decoder);
    decoder_set_byte_decoder(&decoder, BYTE_DECODER_BIT_STUFFING, &byte_assembler);
    decoder_set_frame_decoder(&decoder, FRAME_DECODER_COBS, &cobs_decoder);

    // Main processing loop
    LOG_INFO("Starting main processing loop");
    while (decoder_busy(&decoder))
    {
        decoder_task(&decoder);
    }

    // Send 0xAA in the form of alternating sine waves of 1100 and 2200Hz
    for (int j = 0; j < 4; j++)
    {
        uint16_t sample_buffer[fsk_timing.samples_per_bit];
        generate_sine_wave(sample_buffer, FQ0, fsk_timing.sample_rate, sizeof(sample_buffer) / sizeof(sample_buffer[0]));
        decoder_process_samples(&decoder, sample_buffer, fsk_timing.samples_per_bit); // Placeholder for sample input

        // Make sure samples are processed
        while (decoder_busy(&decoder))
        {
            decoder_task(&decoder);
        }

        generate_sine_wave(sample_buffer, FQ1, fsk_timing.sample_rate, sizeof(sample_buffer) / sizeof(sample_buffer[0]));
        decoder_process_samples(&decoder, sample_buffer, fsk_timing.samples_per_bit); // Placeholder for sample input

        // Make sure samples are processed
        while (decoder_busy(&decoder))
        {
            decoder_task(&decoder);
        }
    }

failed:
    return ret;
}