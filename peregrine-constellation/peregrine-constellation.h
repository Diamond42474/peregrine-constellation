#ifndef PEREGRINE_CONSTELLATION
#define PEREGRINE_CONSTELLATION

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include "include/pconfig.h"

// Error codes
typedef enum {
    PC_SUCCESS = 0,
    PC_ERROR_INVALID_HANDLE = -1,
    PC_ERROR_INVALID_PAYLOAD_LENGTH = -2,
    PC_ERROR_SEND_FUNCTION_NOT_SET = -3,
    PC_ERROR_LINE_BUSY = -4
} pc_error_t;

// Callback type for receiving messages
typedef void (*pc_message_received_callback_t)(const uint8_t *payload, size_t payload_length);
typedef int (*pc_line_busy_func)(void);
typedef int (*pc_send_func)(const uint8_t *data, size_t length);


// Peregrine Constellation handle
typedef struct {
    uint16_t node_address;

    pc_message_received_callback_t callback;
    pc_send_func send_func;
    pc_line_busy_func line_busy_func;

    // broadcasting settings
    uint8_t retry_count;
    uint8_t max_transmit_retries;
    uint32_t backoff_base_time_ms;
    uint32_t backoff_max_time_ms;
    uint32_t last_retry_time_ms;

    uint8_t rx_buffer[pconfigRX_BUFFER_SIZE];
    uint8_t tx_buffer[pconfigRX_BUFFER_SIZE][pconfigMAX_PAYLOAD_SIZE];
}pc_handle_t;

// Initialize the library with the node address
pc_error_t pc_init(pc_handle_t *handle);
pc_error_t pc_config_node_address(pc_handle_t *handle, uint16_t node_address);
pc_error_t pc_config_received_callback(pc_handle_t *handle, pc_message_received_callback_t callback);
pc_error_t pc_config_tx_func(pc_handle_t *handle, pc_send_func send_func);
pc_error_t pc_config_line_busy_func(pc_handle_t *handle, pc_line_busy_func line_busy_func);

// Send a message
pc_error_t pc_send_message(pc_handle_t *handle, uint16_t dest_addr, const uint8_t *payload, size_t payload_length);

// Process incoming data (should be called with received data)
void pc_process_incoming_data(pc_handle_t *handle, const uint8_t *data, size_t length);

// Update function to be called periodically to handle retries
void pc_update(pc_handle_t *handle, uint32_t current_time_ms);

#endif