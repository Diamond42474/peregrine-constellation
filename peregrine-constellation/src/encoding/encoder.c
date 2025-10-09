#include "encoder.h"

#include "cobs_encoder.h"

#include "c-logger.h"

int encoder_init(encoder_handle_t *handle)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Invalid argument: handle is NULL");
        ret = -1;
        goto failed;
    }

    handle->type = ENCODER_TYPE_NONE;

failed:
    return ret;
}

int encoder_deinit(encoder_handle_t *handle)
{
    int ret = 0;
    if (!handle)
    {
        LOG_ERROR("Invalid argument: handle is NULL");
        ret = -1;
        goto failed;
    }

    handle->type = ENCODER_TYPE_NONE;

failed:
    return ret;
}

int encoder_set_type(encoder_handle_t *handle, encoder_type_e type)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Invalid argument: handle is NULL");
        ret = -1;
        goto failed;
    }

    handle->type = type;

failed:
    return ret;
}

encoder_type_e encoder_get_type(encoder_handle_t *handle)
{
    if (!handle)
    {
        LOG_ERROR("Invalid argument: handle is NULL");
        return ENCODER_TYPE_NONE;
    }

    return handle->type;
}

int encoder_encode(encoder_handle_t *handle, const unsigned char *input, size_t input_len, unsigned char *output, size_t output_len)
{
    int ret = 0;

    if (!handle || !input || !output)
    {
        LOG_ERROR("Invalid argument: handle, input, or output is NULL");
        ret = -1;
        goto failed;
    }

    switch (handle->type)
    {
    ENCODER_TYPE_NONE:
        LOG_WARN("Encoder type is NONE, no encoding performed");
        ret = -1;
        goto failed;
        break;
    ENCODER_TYPE_COBS:
        if (cobs_encode(input, input_len, output, output_len))
        {
            LOG_ERROR("COBS encoding failed");
            ret = -1;
            goto failed;
        }
        break;
    default:
        LOG_ERROR("Unsupported encoder type: %d", handle->type);
        ret = -1;
        goto failed;
    }

failed:
    return ret;
}