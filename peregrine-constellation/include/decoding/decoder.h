#ifndef DECODER_H
#define DECODER_H

#include "fsk_decoder.h"
#include "byte_assembler.h"
#include "cobs_decoder.h"

typedef enum
{
    MESSAGE_DECODER_NONE,
    MESSAGE_DECODER_COBS,
} message_decoder_e;

typedef enum
{
    BYTE_DECODER_NONE,
    BYTE_DECODER_BIT_STUFFING,
} byte_decoder_e;

typedef enum
{
    BIT_DECODER_NONE,
    BIT_DECODER_FSK,
} bit_decoder_e;

typedef enum
{
    DECODER_STATE_UNINITIALIZED,
    DECODER_STATE_INITIALIZING,
    DECODER_STATE_IDLE,
    DECODER_STATE_DECODING,
} decoder_state_t;

typedef struct
{
    bit_decoder_e bit_decoder;
    byte_decoder_e byte_decoder;
    message_decoder_e message_decoder;

    void *bit_decoder_handle;
    void *byte_decoder_handle;
    void *message_decoder_handle;

    decoder_state_t state;
} decoder_handle_t;

int decoder_init(decoder_handle_t *handle);
int decoder_deinit(decoder_handle_t *handle);

int decoder_set_message_decoder(decoder_handle_t *handle, message_decoder_e type, void *message_decoder_handle);
int decoder_set_byte_decoder(decoder_handle_t *handle, byte_decoder_e type, void *byte_decoder_handle);
int decoder_set_bit_decoder(decoder_handle_t *handle, bit_decoder_e type, void *bit_decoder_handle);

int decoder_task(decoder_handle_t *handle);

int decoder_process_samples(decoder_handle_t *handle, const uint16_t *samples, size_t num_samples);
bool decoder_has_message(decoder_handle_t *handle);
int decoder_get_message(decoder_handle_t *handle, unsigned char *buffer, size_t buffer_len, size_t *message_len);

#endif // DECODER_H