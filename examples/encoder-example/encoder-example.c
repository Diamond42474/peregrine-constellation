#include "encoding/encoder.h"
#include "c-logger.h"

int main(void)
{
    int ret = 0;

    log_init(LOG_LEVEL_DEBUG);

    LOG_INFO("Starting encoder example");

    encoder_handle_t encoder;
    if (encoder_init(&encoder))
    {
        LOG_ERROR("Failed to initialize encoder");
        ret = -1;
        goto failed;
    }

    encoder_set_input_size(&encoder, 1024);
    encoder_set_output_size(&encoder, 1024);
    encoder_set_type(&encoder, ENCODER_TYPE_COBS);

    // Startup
    while (encoder_busy(&encoder))
    {
        encoder_task(&encoder);
    }

    char data[] = "Hello, World!";
    encoder_write(&encoder, (unsigned char *)data, sizeof(data));
    encoder_flush(&encoder);
    while (encoder_busy(&encoder))
    {
        encoder_task(&encoder);
    }

    // Read encoded bits
    LOG_INFO("Encoded bits:");
    int bit_count = 0;
    while (encoder_data_available(&encoder))
    {
        bool bit;
        encoder_read(&encoder, &bit);
        if (bit)
        {
            printf("1");
        }
        else
        {
            printf("0");
        }
        if (bit_count >= 7)
        {
            printf(" ");
            bit_count = 0;
        }
        else
        {
            bit_count++;
        }
    }
    printf("\n");

    // Ensure all tasks are complete before exiting
    while (encoder_busy(&encoder))
    {
        encoder_task(&encoder);
    }

failed:
    return ret;
}