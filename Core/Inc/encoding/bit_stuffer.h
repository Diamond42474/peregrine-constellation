#ifndef BIT_STUFFER_H
#define BIT_STUFFER_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_CONSECUTIVE (5)

typedef struct
{
    bool initialized;

    bool last_bit;
    uint8_t consecutive_count;

    bool emit_stuffed_bit;
} bit_stuffer_t;

typedef struct
{
    bool initialized;

    bool last_bit;
    uint8_t consecutive_count;

    bool discard_next_bit;
} bit_unstuffer_t;

static inline void bit_stuffer_init(bit_stuffer_t *s)
{
    s->initialized = false;
    s->last_bit = false;
    s->consecutive_count = 0;
    s->emit_stuffed_bit = false;
}

static inline void bit_unstuffer_init(bit_unstuffer_t *u)
{
    u->initialized = false;
    u->last_bit = false;
    u->consecutive_count = 0;
    u->discard_next_bit = false;
}

static inline void bit_stuffer_reset(bit_stuffer_t *s)
{
    s->initialized = false;
    s->last_bit = false;
    s->consecutive_count = 0;
    s->emit_stuffed_bit = false;
}

static inline void bit_unstuffer_reset(bit_unstuffer_t *u)
{
    u->initialized = false;
    u->last_bit = false;
    u->consecutive_count = 0;
    u->discard_next_bit = false;
}

static inline int bit_stuffer_process(
    bit_stuffer_t *s,
    bool input,
    bool *output,
    bool *consumed_input)
{
    // Emit stuffed bit first
    if(s->emit_stuffed_bit)
    {
        *output = !s->last_bit;
        *consumed_input = false;

        s->last_bit = *output;
        s->consecutive_count = 1;

        s->emit_stuffed_bit = false;

        return 0;
    }

    *output = input;
    *consumed_input = true;

    if(!s->initialized)
    {
        s->initialized = true;
        s->last_bit = input;
        s->consecutive_count = 1;
        return 0;
    }

    if(input == s->last_bit)
    {
        s->consecutive_count++;
    }
    else
    {
        s->last_bit = input;
        s->consecutive_count = 1;
    }

    if(s->consecutive_count == MAX_CONSECUTIVE)
    {
        s->emit_stuffed_bit = true;
    }

    return 0;
}

static inline int bit_unstuffer_process(
    bit_unstuffer_t *u,
    bool input,
    bool *output,
    bool *produced_output)
{
    // Potential stuffed bit
    if(u->discard_next_bit)
    {
        u->discard_next_bit = false;

        // Opposite bit = stuffed bit
        if(input != u->last_bit)
        {
            u->last_bit = input;
            u->consecutive_count = 1;

            *produced_output = false;
            return 0;
        }
    }

    *output = input;
    *produced_output = true;

    if(!u->initialized)
    {
        u->initialized = true;
        u->last_bit = input;
        u->consecutive_count = 1;
        return 0;
    }

    if(input == u->last_bit)
    {
        u->consecutive_count++;
    }
    else
    {
        u->last_bit = input;
        u->consecutive_count = 1;
    }

    if(u->consecutive_count == MAX_CONSECUTIVE)
    {
        u->discard_next_bit = true;
    }

    return 0;
}

#endif // BIT_STUFFER_H