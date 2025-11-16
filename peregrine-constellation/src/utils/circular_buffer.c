#include "circular_buffer.h"

#include <stdlib.h>
#include <string.h>
#include "c-logger.h"

/**
 * @brief Initialize a circular buffer with a static buffer.
 *
 * @param cb Pointer to the circular buffer handle.
 * @param buffer Pointer to the static buffer memory.
 * @param element_size Size of each element in the buffer.
 * @param max Maximum number of elements in the buffer.
 *
 * @return error code: 0 = successful, -1 = failed
 */
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

/**
 * @brief Initialize a circular buffer with dynamic memory allocation.
 *
 * @param cb Pointer to the circular buffer handle.
 * @param element_size Size of each element in the buffer.
 * @param max Maximum number of elements in the buffer.
 *
 * @return error code: 0 = successful, -1 = failed
 */
int circular_buffer_dynamic_init(circular_buffer_t *cb, size_t element_size, size_t max)
{
    if (!cb || element_size == 0 || max == 0)
    {
        LOG_ERROR("Invalid parameters for circular buffer dynamic init");
        return -1; // Invalid parameters
    }

    LOG_DEBUG("Allocating %zu bytes for circular buffer", element_size * max);
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

size_t circular_buffer_count(circular_buffer_t *cb)
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

    // Maintain head position incase it is used for DMA operations
    cb->tail = cb->head;
    cb->full = false;
    cb->count = 0;
}

/**
 * @brief Set the head index of the circular buffer.
 *
 * @details This function manually sets the head index of the circular buffer.
 * It is recommended that this function is used with caution, as it can lead to data corruption.
 * It is built for use with DMA operations where the head index needs to be updated externally.
 *
 * @param cb Pointer to the circular buffer handle.
 * @param index New head index to set.
 *
 * @return error code: 0 = successful, -1 = failed
 */
int circular_buffer_set_head(circular_buffer_t *cb, size_t index)
{
    if (!cb)
    {
        LOG_ERROR("Handle is NULL");
        return -1; // Invalid parameter
    }

    if (index >= cb->max)
    {
        LOG_ERROR("Index out of bounds for circular buffer set head");
        return -1; // Index out of bounds
    }

    cb->head = index;
    cb->full = (cb->head == cb->tail);
    
    // Recalculate count
    if (cb->full)
    {
        cb->count = cb->max;
    }
    else if (cb->head >= cb->tail)
    {
        cb->count = cb->head - cb->tail;
    }
    else
    {
        cb->count = cb->max + cb->head - cb->tail;
    }

    return 0; // Success
}