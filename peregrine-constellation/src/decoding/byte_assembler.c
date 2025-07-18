#include "byte_assembler.h"

#include <stdlib.h>
#include <stdint.h>

static unsigned char current_byte = 0;
static int bits_collected = 0;
static bit_order_t bit_order = BIT_ORDER_LSB_FIRST;
static void (*byte_callback)(unsigned char byte) = NULL;
static bool initialized = false;

int byte_assembler_init(void)
{
    current_byte = 0;
    bits_collected = 0;
    bit_order = BIT_ORDER_LSB_FIRST;
    byte_callback = NULL;
    initialized = true;
    return 0;
}

int byte_assembler_deinit(void)
{
    initialized = false;
    current_byte = 0;
    bits_collected = 0;
    byte_callback = NULL;
    return 0;
}

int byte_assembler_set_byte_callback(void (*callback)(unsigned char byte))
{
    if (!initialized)
    {
        return -1;
    }

    byte_callback = callback;
    return 0;
}

int byte_assembler_set_bit_order(bit_order_t order)
{
    if (!initialized)
    {
        return -1;
    }

    bit_order = order;
    return 0;
}

int byte_assembler_reset(void)
{
    if (!initialized)
    {
        return -1;
    }

    current_byte = 0;
    bits_collected = 0;
    return 0;
}

int byte_assembler_bits_collected(void)
{
    return bits_collected;
}

void byte_assembler_bit_callback(int bit)
{
    if (!initialized)
    {
        return;
    }

    if (bit != 0 && bit != 1)
    {
        return;
    } // ignore invalid bits

    if (bit_order == BIT_ORDER_LSB_FIRST)
    {
        current_byte |= (bit & 0x01) << bits_collected;
    }
    else // MSB first
    {
        current_byte |= (bit & 0x01) << (7 - bits_collected);
    }

    bits_collected++;

    if (bits_collected >= 8)
    {
        if (byte_callback)
        {
            byte_callback(current_byte);
        }

        // reset for next byte
        current_byte = 0;
        bits_collected = 0;
    }
}
