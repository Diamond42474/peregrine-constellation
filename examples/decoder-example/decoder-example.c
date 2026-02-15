#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "decoding/decoder.h"
#include "c-logger.h"

#include "decoding/fsk_decoder.h"
#include "decoding/byte_assembler.h"

#include "utils/fsk_utils.h"

#define FQ0 1100.0f
#define FQ1 2200.0f
#define BAUD_RATE 32

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

void send_byte(decoder_handle_t *decoder, unsigned char byte, size_t sample_size, float sample_rate)
{
    uint16_t buffer[sample_size];
    for (int bit = 7; bit >= 0; bit--)
    {
        if ((byte >> bit) & 1)
        {
            generate_sine_wave(buffer, FQ1, sample_rate, sample_size);
        }
        else
        {
            generate_sine_wave(buffer, FQ0, sample_rate, sample_size);
        }
        decoder_process_samples(decoder, buffer, sample_size);
        while (decoder_busy(decoder))
        {
            decoder_task(decoder);
        }
    }
}

int main(void)
{
    int ret = 0;

    log_init(LOG_LEVEL_INFO);

    decoder_handle_t decoder;
    fsk_decoder_handle_t fsk_decoder;
    byte_assembler_handle_t byte_assembler;

    // Calculate FSK parameters
    fsk_timing_t fsk_timing = fsk_calculate_timing(FQ0, FQ1, BAUD_RATE);
    printf("Recommended Sample Rate: %.2ld Hz\n", fsk_timing.sample_rate);
    printf("Samples per Bit: %zu\n", fsk_timing.samples_per_bit);

    // Initialize FSK Decoder
    fsk_decoder_init(&fsk_decoder);
    fsk_decoder_set_rates(&fsk_decoder, (int)fsk_timing.samples_per_bit, (int)fsk_timing.sample_rate);
    fsk_decoder_set_frequencies(&fsk_decoder, FQ0, FQ1);
    fsk_decoder_set_power_threshold(&fsk_decoder, 1000.0f);
    // Initialize Byte Assembler
    byte_assembler_init(&byte_assembler);
    if (byte_assembler_set_preamble(&byte_assembler, 0xABBA))
    {
        LOG_ERROR("Failed to set preamble for byte assembler");
        ret = -1;
        goto failed;
    } // Buffer for 256 bytes

    // Initialize Decoder
    decoder_init(&decoder);
    decoder_set_bit_decoder(&decoder, BIT_DECODER_FSK, &fsk_decoder);
    decoder_set_byte_decoder(&decoder, BYTE_DECODER_BIT_STUFFING, &byte_assembler);
    decoder_set_input_buffer_size(&decoder, fsk_timing.samples_per_bit * 128); // Input buffer for samples
    decoder_set_output_buffer_size(&decoder, 10);                              // Output buffer for packets

    // Main processing loop
    LOG_INFO("Starting main processing loop");
    while (decoder_busy(&decoder))
    {
        if (decoder_task(&decoder))
        {
            LOG_ERROR("Decoder task failed");
            ret = -1;
            goto failed;
        }
    }

    // Send half a byte of 0s to test bit alignment mechanism
    for (int i = 0; i < 12; i++)
    {
        uint16_t sample_buffer[fsk_timing.samples_per_bit];
        generate_sine_wave(sample_buffer, FQ0, fsk_timing.sample_rate, sizeof(sample_buffer) / sizeof(sample_buffer[0]));
        decoder_process_samples(&decoder, sample_buffer, fsk_timing.samples_per_bit); // Placeholder for sample input

        // Make sure samples are processed
        while (decoder_busy(&decoder))
        {
            decoder_task(&decoder);
        }
    }

    // Send 0xABBA in the form of alternating sine waves of 1100 and 2200Hz
    send_byte(&decoder, 0xAB, fsk_timing.samples_per_bit, fsk_timing.sample_rate);
    send_byte(&decoder, 0xBA, fsk_timing.samples_per_bit, fsk_timing.sample_rate);
    // send_byte(&decoder, 0x03, fsk_timing.samples_per_bit, fsk_timing.sample_rate);
    // send_byte(&decoder, 0xB1, fsk_timing.samples_per_bit, fsk_timing.sample_rate);
    // send_byte(&decoder, 0x2F, fsk_timing.samples_per_bit, fsk_timing.sample_rate);
    // send_byte(&decoder, 0x00, fsk_timing.samples_per_bit, fsk_timing.sample_rate);
    // send_byte(&decoder, 0x03, fsk_timing.samples_per_bit, fsk_timing.sample_rate);
    // send_byte(&decoder, 0xB1, fsk_timing.samples_per_bit, fsk_timing.sample_rate);
    // send_byte(&decoder, 0x2F, fsk_timing.samples_per_bit, fsk_timing.sample_rate);
    // send_byte(&decoder, 0x00, fsk_timing.samples_per_bit, fsk_timing.sample_rate);
    // send_byte(&decoder, 0x03, fsk_timing.samples_per_bit, fsk_timing.sample_rate);
    // send_byte(&decoder, 0xB1, fsk_timing.samples_per_bit, fsk_timing.sample_rate);
    // send_byte(&decoder, 0x2F, fsk_timing.samples_per_bit, fsk_timing.sample_rate);
    // send_byte(&decoder, 0x00, fsk_timing.samples_per_bit, fsk_timing.sample_rate);
    // send_byte(&decoder, 0x03, fsk_timing.samples_per_bit, fsk_timing.sample_rate);
    // send_byte(&decoder, 0xB1, fsk_timing.samples_per_bit, fsk_timing.sample_rate);
    // send_byte(&decoder, 0x2F, fsk_timing.samples_per_bit, fsk_timing.sample_rate);
    // send_byte(&decoder, 0x00, fsk_timing.samples_per_bit, fsk_timing.sample_rate);

    if (decoder_has_packet(&decoder))
    {
        LOG_INFO("Packet available");
    }
    else
    {
        LOG_INFO("No frame");
    }

failed:
    return ret;
}