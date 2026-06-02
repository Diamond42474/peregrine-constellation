#ifndef PEREGRINE_CONSTELLATION
#define PEREGRINE_CONSTELLATION

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

// Error codes
typedef enum {
    PC_SUCCESS = 0,
    PC_ERROR_INVALID_HANDLE
} pc_error_e;

typedef void (*message_callback_t)(const uint8_t *data, size_t len, uint8_t src_addr);

typedef struct pc_handle pc_handle_t;

// Initialize the library with the node address
pc_handle_t *pc_init(message_callback_t callback);

// Send a message
pc_error_e pc_send_message(pc_handle_t *handle, uint8_t dest_addr, const uint8_t *payload, size_t payload_length);

// Update function to be called periodically to handle retries
void pc_task(pc_handle_t *handle);

#endif
