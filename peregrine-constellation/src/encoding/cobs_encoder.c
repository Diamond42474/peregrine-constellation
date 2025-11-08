#include "cobs_encoder.h"

#include "c-logger.h"

int cobs_encode(const uint8_t *input, size_t input_len, uint8_t *output, size_t output_len)
{
    if (!input || !output)
        return -1;

    size_t read_index = 0;
    size_t write_index = 1; // Reserve space for first code byte
    size_t code_index = 0;  // Position of current code byte
    uint8_t code = 1;

    while (read_index < input_len)
    {
        if (input[read_index] == 0)
        {
            // Write code byte for current block
            output[code_index] = code;
            code = 1;
            code_index = write_index++;
            if (write_index > output_len)
                return -1; // Output buffer overflow
            read_index++;
        }
        else
        {
            // Copy non-zero byte
            if (write_index >= output_len)
                return -1; // Output buffer overflow
            output[write_index++] = input[read_index++];
            code++;

            if (code == 0xFF)
            {
                // Max block size reached, write code byte and start new block
                output[code_index] = code;
                code = 1;
                code_index = write_index++;
                if (write_index > output_len)
                    return -1;
            }
        }
    }

    // Write final code byte
    output[code_index] = code;

    // Append frame delimiter zero byte
    if (write_index >= output_len)
        return -1;
    output[write_index++] = 0;

    return (int)write_index;
}

int cobs_encode_cb(circular_buffer_t *input_cb, size_t input_size, circular_buffer_t *output_cb)
{
    int ret = 0;

    if (!input_cb || !output_cb)
    {
        LOG_ERROR("Input or output circular buffer is NULL");
        return -1;
    }

    size_t read_index = 0;
    size_t write_index = 1; // Reserve space for first code byte
    size_t code_index = 0;  // Position of current code byte
    uint8_t code = 1;

    for (size_t i = 0; i < input_size; i++)
    {
        uint8_t byte;
        if (circular_buffer_pop(input_cb, &byte))
        {
            LOG_ERROR("Failed to pop from input circular buffer");
            return -1;
        }

        if (byte == 0)
        {
            // Write code byte for current block
            if (circular_buffer_push(output_cb, &code_index))
            {
                LOG_ERROR("Failed to push to output circular buffer");
                return -1;
            }
            code = 1;
            code_index = write_index++;
            read_index++;
        }
        else
        {
            // Copy non-zero byte
            if (circular_buffer_push(output_cb, &byte))
            {
                LOG_ERROR("Failed to push to output circular buffer");
                return -1;
            }
            write_index++;
            read_index++;
            code++;

            if (code == 0xFF)
            {
                // Max block size reached, write code byte and start new block
                if (circular_buffer_push(output_cb, &code_index))
                {
                    LOG_ERROR("Failed to push to output circular buffer");
                    return -1;
                }
                code = 1;
                code_index = write_index++;
            }
        }
    }

failed:
    return ret;
}