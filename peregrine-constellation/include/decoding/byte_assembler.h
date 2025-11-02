#ifndef BYTE_ASSEMBLER_H
#define BYTE_ASSEMBLER_H

#include <stdbool.h>
#include "circular_buffer.h"

typedef enum
{
    BIT_ORDER_LSB_FIRST,
    BIT_ORDER_MSB_FIRST
} bit_order_t;

typedef enum
{
    BYTE_ASSEMBLER_STATE_UNINITIALIZED,
    BYTE_ASSEMBLER_STATE_INITIALIZING,
    BYTE_ASSEMBLER_STATE_IDLE,
    BYTE_ASSEMBLER_STATE_ASSEMBLING,
} byte_assembler_state_t;

typedef struct
{
    bit_order_t bit_order;
    unsigned char preamble_byte;

    unsigned char preamble_buffer;
    int preamble_bits;
    bool preamble_found;

    unsigned char current_byte;
    int bits_collected;
    
    circular_buffer_t bit_buffer;
    size_t bit_buffer_size;
    circular_buffer_t byte_buffer;
    size_t byte_buffer_size;

    byte_assembler_state_t state;
} byte_assembler_handle_t;

int byte_assembler_init(byte_assembler_handle_t *handle);
int byte_assembler_deinit(byte_assembler_handle_t *handle);

int byte_assembler_set_bit_order(byte_assembler_handle_t *handle, bit_order_t order);
int byte_assembler_set_byte_buffer_size(byte_assembler_handle_t *handle, size_t size);
int byte_assembler_set_bit_buffer_size(byte_assembler_handle_t *handle, size_t size);
int byte_assembler_set_preamble(byte_assembler_handle_t *handle, unsigned char preamble);

int byte_assembler_process_bit(byte_assembler_handle_t *handle, bool bit);
bool byte_assembler_has_byte(byte_assembler_handle_t *handle);
int byte_assembler_get_byte(byte_assembler_handle_t *handle, unsigned char *byte);
int byte_assembler_reset(byte_assembler_handle_t *handle);

int byte_assembler_task(byte_assembler_handle_t *handle);
bool byte_assembler_busy(byte_assembler_handle_t *handle);

#endif // BYTE_ASSEMBLER_H