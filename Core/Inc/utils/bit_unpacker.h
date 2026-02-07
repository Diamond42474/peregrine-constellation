#ifndef BIT_UNPACKER_H
#define BIT_UNPACKER_H

#include "circular_buffer.h"

typedef struct
{
    unsigned char current_byte;
    unsigned char bit_pos;
    bool has_byte;
} bit_unpacker_t;

static inline void bit_unpacker_init(bit_unpacker_t *u)
{
    u->has_byte = false;
    u->bit_pos = 0;
}

static inline int bit_unpacker_pop(bit_unpacker_t *u, circular_buffer_t *cb, bool *bit)
{
    if (!u->has_byte || u->bit_pos == 8)
    {
        if (circular_buffer_pop(cb, &u->current_byte))
        {
            return -1; // no data
        }
        u->bit_pos = 0;
        u->has_byte = true;
    }
    *bit = (u->current_byte >> (7 - u->bit_pos)) & 1;
    u->bit_pos++;
    return 0;
}

#endif // BIT_UNPACKER_H