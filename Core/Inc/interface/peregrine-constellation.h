#ifndef PEREGRINE_CONSTELLATION
#define PEREGRINE_CONSTELLATION

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

// Error codes
typedef enum {
    PC_SUCCESS = 0,
    PC_ERROR_INVALID_HANDLE
} pc_error_t;


typedef struct pc_handle pc_handle_t;

// Initialize the library with the node address
pc_handle_t *pc_init(void);
pc_error_t pc_config_node_address(pc_handle_t *handle, uint8_t node_address);

// Send a message
pc_error_t pc_send_message(pc_handle_t *handle, uint8_t dest_addr, const uint8_t *payload, size_t payload_length);

// Process incoming data (should be called with received data)
void pc_process_incoming(pc_handle_t *handle, const uint16_t *data, size_t length);

// Update function to be called periodically to handle retries
void pc_task(pc_handle_t *handle);

#endif
