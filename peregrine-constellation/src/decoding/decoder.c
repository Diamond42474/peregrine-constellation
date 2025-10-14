#include "decoder.h"

#include "c-logger.h"
#include "fsk_decoder.h"
#include "byte_assembler.h"
#include "cobs_decoder.h"

int decoder_init(decoder_handle_t *handle)
{
    if (!handle)
    {
        LOG_ERROR("Decoder handle is NULL");
        return -1;
    }

    handle->bit_decoder = BIT_DECODER_NONE;
    handle->byte_decoder = BYTE_DECODER_NONE;
    handle->message_decoder = MESSAGE_DECODER_NONE;

    handle->bit_decoder_handle = NULL;
    handle->byte_decoder_handle = NULL;
    handle->message_decoder_handle = NULL;

    handle->state = DECODER_STATE_INITIALIZING;

    return 0;
}

int decoder_deinit(decoder_handle_t *handle)
{
    if (!handle)
    {
        LOG_ERROR("Decoder handle is NULL");
        return -1;
    }

    if (handle->state == DECODER_STATE_UNINITIALIZED)
    {
        LOG_WARN("Decoder is already uninitialized");
        return 0;
    }

    handle->bit_decoder = BIT_DECODER_NONE;
    handle->byte_decoder = BYTE_DECODER_NONE;
    handle->message_decoder = MESSAGE_DECODER_NONE;

    handle->bit_decoder_handle = NULL;
    handle->byte_decoder_handle = NULL;
    handle->message_decoder_handle = NULL;

    handle->state = DECODER_STATE_UNINITIALIZED;

    return 0;
}

int decoder_set_message_decoder(decoder_handle_t *handle, message_decoder_e type, void *message_decoder_handle)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Decoder handle is NULL");
        return -1;
    }

    if (!message_decoder_handle)
    {
        LOG_ERROR("Message decoder handle is NULL");
        ret = -1;
        goto failed;
    }

    handle->message_decoder = type;
    handle->message_decoder_handle = message_decoder_handle;

failed:
    return ret;
}

int decoder_set_byte_decoder(decoder_handle_t *handle, byte_decoder_e type, void *byte_decoder_handle)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Decoder handle is NULL");
        return -1;
    }

    if (!byte_decoder_handle)
    {
        LOG_ERROR("Byte decoder handle is NULL");
        ret = -1;
        goto failed;
    }

    handle->byte_decoder = type;
    handle->byte_decoder_handle = byte_decoder_handle;

failed:
    return ret;
}

int decoder_set_bit_decoder(decoder_handle_t *handle, bit_decoder_e type, void *bit_decoder_handle)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Decoder handle is NULL");
        return -1;
    }

    if (!bit_decoder_handle)
    {
        LOG_ERROR("FSK decoder handle is NULL");
        ret = -1;
        goto failed;
    }

    handle->bit_decoder = type;
    handle->bit_decoder_handle = bit_decoder_handle;

failed:
    return ret;
}

int decoder_task(decoder_handle_t *handle)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Decoder handle is NULL");
        return -1;
    }

    switch (handle->state)
    {
    case DECODER_STATE_UNINITIALIZED:
        LOG_ERROR("Decoder is uninitialized");
        ret = -1;
        goto failed;
        break;
    case DECODER_STATE_INITIALIZING:
        // Initialize bit decoder
        break;
    case DECODER_STATE_IDLE:
        // Check if there are samples to process
        break;
    case DECODER_STATE_DECODING:
        // Process decoding steps
        break;
    default:
        LOG_ERROR("Unknown decoder state");
        handle->state = DECODER_STATE_UNINITIALIZED;
        ret = -1;
        goto failed;
        break;
    }

failed:
    return ret;
}

int decoder_process_samples(decoder_handle_t *handle, const uint16_t *samples, size_t num_samples);
bool decoder_has_message(decoder_handle_t *handle);
int decoder_get_message(decoder_handle_t *handle, unsigned char *buffer, size_t buffer_len, size_t *message_len);