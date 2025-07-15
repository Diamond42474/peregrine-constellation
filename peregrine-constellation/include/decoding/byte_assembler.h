#ifndef BYTE_ASSEMBLER_H
#define BYTE_ASSEMBLER_H

#include <stdbool.h>

int byte_assembler_init(void);
int byte_assembler_deinit(void);

int byte_assembler_set_byte_callback(void (*callback)(unsigned char byte));
void byte_assembler_bit_callback(int bit);

int byte_assembler_reset(void);

typedef enum {
    BIT_ORDER_LSB_FIRST,
    BIT_ORDER_MSB_FIRST
} bit_order_t;

int byte_assembler_set_bit_order(bit_order_t order);

int byte_assembler_bits_collected(void);  // returns 0–7

#endif // BYTE_ASSEMBLER_H