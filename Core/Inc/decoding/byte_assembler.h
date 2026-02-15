#ifndef BYTE_ASSEMBLER_H
#define BYTE_ASSEMBLER_H

#include <stdbool.h>
#include <stdint.h>
#include "utils/circular_buffer.h"
#include "decoding/decoder.h"

typedef struct
{
    uint16_t preamble;

    uint16_t preamble_buffer;
    int preamble_bits;
    bool preamble_found;

    unsigned char current_byte;
    int bits_collected;

    enum
    {
        BYTE_ASSEMBLER_WAITING_FOR_PREAMBLE,
        BYTE_ASSEMBLER_ASSEMBLING,
    } state;
} byte_assembler_handle_t;

int byte_assembler_init(byte_assembler_handle_t *handle);
int byte_assembler_deinit(byte_assembler_handle_t *handle);

int byte_assembler_set_byte_buffer_size(byte_assembler_handle_t *handle, size_t size);
int byte_assembler_set_bit_buffer_size(byte_assembler_handle_t *handle, size_t size);
int byte_assembler_set_preamble(byte_assembler_handle_t *handle, uint16_t preamble);

int byte_assembler_process_bit(byte_assembler_handle_t *handle, decoder_handle_t *ctx, bool bit);
int byte_assembler_reset(byte_assembler_handle_t *handle);

#endif // BYTE_ASSEMBLER_H