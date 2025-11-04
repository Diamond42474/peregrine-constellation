#include "encoder.h"
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

    while (encoder_busy(&encoder))
    {
        encoder_task(&encoder);
    }

failed:
    return ret;
}