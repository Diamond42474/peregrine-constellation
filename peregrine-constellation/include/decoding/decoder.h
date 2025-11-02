#ifndef DECODER_H
#define DECODER_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "circular_buffer.h"

typedef enum
{
    FRAME_DECODER_NONE,
    FRAME_DECODER_COBS,
} frame_decoder_e;

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

typedef struct
{
    bit_decoder_e bit_decoder;
    byte_decoder_e byte_decoder;
    frame_decoder_e frame_decoder;

    void *bit_decoder_handle;
    void *byte_decoder_handle;
    void *frame_decoder_handle;

    enum
    {
        DECODER_STATE_UNINITIALIZED,
        DECODER_STATE_INITIALIZING,
        DECODER_STATE_IDLE,
        DECODER_STATE_PROCESSING,
        DECODER_STATE_TRANSFERRING,
    } state;
} decoder_handle_t;

int decoder_init(decoder_handle_t *handle);
int decoder_deinit(decoder_handle_t *handle);

int decoder_set_frame_decoder(decoder_handle_t *handle, frame_decoder_e type, void *frame_decoder_handle);
int decoder_set_byte_decoder(decoder_handle_t *handle, byte_decoder_e type, void *byte_decoder_handle);
int decoder_set_bit_decoder(decoder_handle_t *handle, bit_decoder_e type, void *bit_decoder_handle);
int decoder_task(decoder_handle_t *handle);

int decoder_process_samples(decoder_handle_t *handle, const uint16_t *samples, size_t num_samples);
bool decoder_has_frame(decoder_handle_t *handle);
int decoder_get_frame(decoder_handle_t *handle, unsigned char *buffer, size_t buffer_len, size_t *frame_len);
bool decoder_busy(decoder_handle_t *handle);

#endif // DECODER_H