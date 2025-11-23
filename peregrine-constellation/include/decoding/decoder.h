#ifndef DECODER_H
#define DECODER_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "circular_buffer.h"
#include "packet_decoder.h"

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

    void *bit_decoder_handle;
    void *byte_decoder_handle;
    packet_decoder_t packet_decoder;

    circular_buffer_t input_buffer;
    size_t input_buffer_size;
    circular_buffer_t output_buffer;
    size_t output_buffer_size;

    enum
    {
        DECODER_STATE_UNINITIALIZED,
        DECODER_STATE_INITIALIZING,
        DECODER_STATE_IDLE,
        DECODER_STATE_PROCESSING,
    } state;
} decoder_handle_t;

int decoder_init(decoder_handle_t *handle);
int decoder_deinit(decoder_handle_t *handle);

int decoder_set_byte_decoder(decoder_handle_t *handle, byte_decoder_e type, void *byte_decoder_handle);
int decoder_set_bit_decoder(decoder_handle_t *handle, bit_decoder_e type, void *bit_decoder_handle);
int decoder_set_input_buffer_size(decoder_handle_t *handle, size_t size);
int decoder_set_output_buffer_size(decoder_handle_t *handle, size_t size);
int decoder_task(decoder_handle_t *handle);

int decoder_process_samples(decoder_handle_t *handle, const uint16_t *samples, size_t num_samples);
int decoder_process_bit(decoder_handle_t *handle, bool bit);
int decoder_process_byte(decoder_handle_t *handle, unsigned char byte);
int decoder_process_packet(decoder_handle_t *handle, packet_t *packet);

bool decoder_has_packet(decoder_handle_t *handle);
int decoder_get_packet(decoder_handle_t *handle, packet_t *packet);
bool decoder_busy(decoder_handle_t *handle);

#endif // DECODER_H