#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <stddef.h>
#include <stdbool.h>

typedef struct
{
    void *buffer;        // Pointer to the buffer memory
    size_t element_size; // Size of each element in the buffer
    size_t count;       // Current number of elements in the buffer
    size_t head;         // Index of the head (next write position)
    size_t tail;         // Index of the tail (next read position)
    size_t max;          // Maximum number of elements in the buffer
    bool full;           // Flag to indicate if the buffer is full
    bool dynamic;        // Flag to indicate if the buffer was dynamically allocated
} circular_buffer_t;

int circular_buffer_static_init(circular_buffer_t *cb, void *buffer, size_t element_size, size_t max);
int circular_buffer_dynamic_init(circular_buffer_t *cb, size_t element_size, size_t max);
int circular_buffer_deinit(circular_buffer_t *cb);

int circular_buffer_push(circular_buffer_t *cb, const void *item);
int circular_buffer_pop(circular_buffer_t *cb, void *item);
int circular_buffer_peek(circular_buffer_t *cb, void *item);
size_t circular_buffer_count(circular_buffer_t *cb);
size_t circular_buffer_capacity(circular_buffer_t *cb);
bool circular_buffer_is_full(circular_buffer_t *cb);
bool circular_buffer_is_empty(circular_buffer_t *cb);
void circular_buffer_reset(circular_buffer_t *cb);
int circular_buffer_set_head(circular_buffer_t *cb, size_t index);

#endif // CIRCULAR_BUFFER_H