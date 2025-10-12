#include "circular_buffer.h"

#include <stdlib.h>
#include <string.h>
#include "c-logger.h"

int circular_buffer_static_init(circular_buffer_t *cb, void *buffer, size_t element_size, size_t max)
{
    if (!cb || !buffer || element_size == 0 || max == 0)
    {
        LOG_ERROR("Invalid parameters for circular buffer static init");
        return -1; // Invalid parameters
    }

    cb->buffer = buffer;
    cb->element_size = element_size;
    cb->max = max;
    cb->head = 0;
    cb->tail = 0;
    cb->full = false;
    cb->dynamic = false;
    cb->count = 0;

    return 0; // Success
}

int circular_buffer_dynamic_init(circular_buffer_t *cb, size_t element_size, size_t max)
{
    if (!cb || element_size == 0 || max == 0)
    {
        LOG_ERROR("Invalid parameters for circular buffer dynamic init");
        return -1; // Invalid parameters
    }

    cb->buffer = malloc(element_size * max);
    if (!cb->buffer)
    {
        LOG_ERROR("Failed to allocate memory for circular buffer");
        return -1; // Memory allocation failed
    }

    cb->element_size = element_size;
    cb->max = max;
    cb->head = 0;
    cb->tail = 0;
    cb->full = false;
    cb->dynamic = true;
    cb->count = 0;

    return 0; // Success
}

int circular_buffer_deinit(circular_buffer_t *cb)
{
    if (!cb)
    {
        LOG_ERROR("Handle is NULL");
        return -1; // Invalid parameter
    }

    if (cb->dynamic && cb->buffer)
    {
        free(cb->buffer);
        cb->buffer = NULL;
    }

    cb->element_size = 0;
    cb->max = 0;
    cb->head = 0;
    cb->tail = 0;
    cb->full = false;
    cb->dynamic = false;
    cb->count = 0;

    return 0; // Success
}

int circular_buffer_push(circular_buffer_t *cb, const void *item)
{
    if (!cb || !item || cb->element_size == 0 || cb->max == 0)
    {
        LOG_ERROR("Invalid parameters for circular buffer push");
        return -1; // Invalid parameters
    }

    memcpy((char *)cb->buffer + (cb->head * cb->element_size), item, cb->element_size);

    if (cb->full)
    {
        cb->tail = (cb->tail + 1) % cb->max; // Overwrite the oldest data
    }

    cb->head = (cb->head + 1) % cb->max;
    cb->full = (cb->head == cb->tail);
    if (!cb->full)
    {
        cb->count++;
    }

    return 0; // Success
}

int circular_buffer_pop(circular_buffer_t *cb, void *item)
{
    if (!cb || !item || cb->element_size == 0 || cb->max == 0 || circular_buffer_is_empty(cb))
    {
        LOG_ERROR("Invalid parameters for circular buffer pop or buffer is empty");
        return -1; // Invalid parameters or buffer is empty
    }

    memcpy(item, (char *)cb->buffer + (cb->tail * cb->element_size), cb->element_size);
    cb->full = false;
    cb->tail = (cb->tail + 1) % cb->max;
    cb->count--;

    return 0; // Success
}

int circular_buffer_peek(circular_buffer_t *cb, void *item)
{
    if (!cb || !item || cb->element_size == 0 || cb->max == 0 || circular_buffer_is_empty(cb))
    {
        LOG_ERROR("Invalid parameters for circular buffer peek or buffer is empty");
        return -1; // Invalid parameters or buffer is empty
    }

    memcpy(item, (char *)cb->buffer + (cb->tail * cb->element_size), cb->element_size);

    return 0; // Success
}

size_t circular_buffer_size(circular_buffer_t *cb)
{
    if (!cb)
    {
        LOG_ERROR("Handle is NULL");
        return 0; // Invalid parameter
    }

    return cb->count;
}

size_t circular_buffer_capacity(circular_buffer_t *cb)
{
    if (!cb)
    {
        LOG_ERROR("Handle is NULL");
        return 0; // Invalid parameter
    }

    return cb->max;
}

bool circular_buffer_is_full(circular_buffer_t *cb)
{
    if (!cb)
    {
        LOG_ERROR("Handle is NULL");
        return false; // Invalid parameter
    }

    return cb->full;
}

bool circular_buffer_is_empty(circular_buffer_t *cb)
{
    if (!cb)
    {
        LOG_ERROR("Handle is NULL");
        return true; // Invalid parameter
    }

    return (!cb->full && (cb->head == cb->tail));
}

void circular_buffer_reset(circular_buffer_t *cb)
{
    if (!cb)
    {
        LOG_ERROR("Handle is NULL");
        return; // Invalid parameter
    }

    cb->head = 0;
    cb->tail = 0;
    cb->full = false;
    cb->count = 0;
}