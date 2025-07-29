#include "cobs_decoder.h"

#include <string.h>
#include <stdbool.h>

#include "c-logger.h"

#define FRAME_BUFFER_SIZE 256

static bool initialized = false;

static uint8_t frame_buffer[FRAME_BUFFER_SIZE];
static size_t write_index = 0;

static uint8_t code = 0;
static uint8_t remaining = 0;

static void (*frame_callback)(const uint8_t *, size_t) = NULL;

int cobs_decoder_set_data_callback(void (*data_callback)(const uint8_t *data, size_t len))
{
    frame_callback = data_callback;
    return 0;
}

int cobs_decoder_init(void)
{
    if (initialized)
    {
        // Already initialized
        return 0;
    }

    write_index = 0;
    code = 0;
    remaining = 0;

    initialized = true;

    return 0;
}

int cobs_decoder_deinit(void)
{
    if (!initialized)
    {
        // Already not initialized
        return 0;
    }

    frame_callback = NULL;
    initialized = false;
    return 0;
}

int cobs_decoder_reset(void)
{
    if (!initialized)
    {
        if (cobs_decoder_init())
        {
            return -1; // Initialization failed
        }
    }

    write_index = 0;
    code = 0;
    remaining = 0;
}

void cobs_decoder_input(uint8_t byte)
{
    if (!initialized)
    {
        if (cobs_decoder_init())
        {
            return; // Initialization failed
        }
    }

    LOG_INFO("COBS Decoder Input: %02X", byte);

    if (code == 0)
    {
        // Start of a new block: code byte indicates how many bytes follow
        if (byte == 0)
        {
            LOG_INFO("COBS Decoder: Zero byte received, resetting decoder");
            // End of frame delimiter
            if (write_index > 0 && frame_callback)
            {
                frame_callback(frame_buffer, write_index);
            } else
            {
                LOG_ERROR("COBS Decoder: No data to process, resetting decoder");
            }
            // Reset for next frame
            cobs_decoder_reset();
            return;
        }

        code = byte;
        remaining = code - 1;

        if (write_index + remaining >= FRAME_BUFFER_SIZE)
        {
            LOG_ERROR("COBS Decoder: Frame buffer overflow, resetting decoder");
            // Overflow - reset
            cobs_decoder_reset();
            return;
        }

        return;
    }

    // Copy next data byte
    frame_buffer[write_index++] = byte;
    remaining--;

    if (remaining == 0)
    {
        // If code < 0xFF, insert a zero byte after this block (except if it's the last block before delimiter)
        if (code < 0xFF && write_index < FRAME_BUFFER_SIZE)
        {
            frame_buffer[write_index++] = 0;
        }
        code = 0; // Ready for next code byte
    }
}
