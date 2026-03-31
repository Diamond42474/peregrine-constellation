#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
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

void generate_noise(uint16_t *buffer, float sample_rate, uint32_t sample_count)
{
    for (uint32_t i = 0; i < sample_count; i++)
    {
        buffer[i] = rand() % (32768); // Random value between 0 and 32767
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
    double sample_rate = calculate_sample_rate(FQ0, FQ1, BAUD_RATE);
    size_t samples_per_bit = sample_rate / BAUD_RATE;
    printf("Recommended Sample Rate: %.2f Hz\n", sample_rate);
    printf("Samples per Bit: %zu\n", samples_per_bit);

    // Initialize FSK Decoder
    fsk_decoder_init(&fsk_decoder);
    fsk_decoder_set_symbol_sample_size(&fsk_decoder, samples_per_bit, 3); // Buffer for 3 symbols to allow for timing recovery
    fsk_decoder_set_sample_rate(&fsk_decoder, sample_rate);
    fsk_decoder_set_frequencies(&fsk_decoder, FQ0, FQ1);
    fsk_decoder_set_power_threshold(&fsk_decoder, 1E13f);
    // Initialize Byte Assembler
    byte_assembler_init(&byte_assembler);
    if (byte_assembler_set_preamble(&byte_assembler, 0xABBA))
    {
        LOG_ERROR("Failed to set preamble for byte assembler");
        ret = -1;
        return ret;
    } // Buffer for 256 bytes

    // Initialize Decoder
    decoder_init(&decoder);
    decoder_set_bit_decoder(&decoder, BIT_DECODER_FSK, &fsk_decoder);
    decoder_set_byte_decoder(&decoder, BYTE_DECODER_BIT_STUFFING, &byte_assembler);
    decoder_set_input_buffer_size(&decoder, samples_per_bit * 5); // Input buffer for samples
    decoder_set_output_buffer_size(&decoder, 10);                 // Output buffer for packets

    // Main processing loop
    LOG_INFO("Startup loop");
    while (decoder_busy(&decoder))
    {
        if (decoder_task(&decoder))
        {
            LOG_ERROR("Decoder task failed");
            ret = -1;
            return ret;
        }
    }

    // Send < samples_per_bit to test timing recovery mechanism
    LOG_INFO("===== Sending noise signal to test timing recovery mechanism =====");
    uint16_t sample_buffer[samples_per_bit * 5];
    for (int i = 0; i < 5; i++)
    {
        generate_noise(sample_buffer, sample_rate, samples_per_bit);
        decoder_process_samples(&decoder, sample_buffer, samples_per_bit); // Placeholder for sample input
        while (decoder_busy(&decoder))
        {
            decoder_task(&decoder);
        }
    }
    generate_noise(sample_buffer, sample_rate, 314);
    decoder_process_samples(&decoder, sample_buffer, 314); // Placeholder for sample input
    while (decoder_busy(&decoder))
    {
        decoder_task(&decoder);
    }

    LOG_INFO("===== Sending single bit to test bit alignment =====");
    generate_sine_wave(sample_buffer, FQ0, sample_rate, 825);
    for (int i = 0; i < samples_per_bit / 5; i++)
    {
        decoder_process_samples(&decoder, sample_buffer + i * 5, 5); // Placeholder for sample input
        while (decoder_busy(&decoder))
        {
            decoder_task(&decoder);
        }
    }

    LOG_INFO("===== Sending test signal (0xABBA) using FSK modulation =====");
    LOG_INFO("Current samples in buffer: %d", circular_buffer_count(&decoder.input_buffer));
    send_byte(&decoder, 0xAB, samples_per_bit, sample_rate);
    send_byte(&decoder, 0xBA, samples_per_bit, sample_rate);
    // send_byte(&decoder, 0x03, samples_per_bit, sample_rate);
    // send_byte(&decoder, 0xB1, samples_per_bit, sample_rate);
    // send_byte(&decoder, 0x2F, samples_per_bit, sample_rate);
    // send_byte(&decoder, 0x00, samples_per_bit, sample_rate);
    // send_byte(&decoder, 0x03, samples_per_bit, sample_rate);
    // send_byte(&decoder, 0xB1, samples_per_bit, sample_rate);
    // send_byte(&decoder, 0x2F, samples_per_bit, sample_rate);
    // send_byte(&decoder, 0x00, samples_per_bit, sample_rate);
    // send_byte(&decoder, 0x03, samples_per_bit, sample_rate);
    // send_byte(&decoder, 0xB1, samples_per_bit, sample_rate);
    // send_byte(&decoder, 0x2F, samples_per_bit, sample_rate);
    // send_byte(&decoder, 0x00, samples_per_bit, sample_rate);
    // send_byte(&decoder, 0x03, samples_per_bit, sample_rate);
    // send_byte(&decoder, 0xB1, samples_per_bit, sample_rate);
    // send_byte(&decoder, 0x2F, samples_per_bit, sample_rate);
    // send_byte(&decoder, 0x00, samples_per_bit, sample_rate);

    // Simulate no signal
    LOG_INFO("===== Simulating noise at end of transmission =====");
    for (int i = 0; i < 10; i++)
    {
        generate_noise(sample_buffer, sample_rate, samples_per_bit);
        decoder_process_samples(&decoder, sample_buffer, samples_per_bit);
        while (decoder_busy(&decoder))
        {
            decoder_task(&decoder);
        }
    }

    if (decoder_has_packet(&decoder))
    {
        LOG_INFO("Packet available");
    }
    else
    {
        LOG_INFO("No packet available");
    }

failed:
    return ret;
}