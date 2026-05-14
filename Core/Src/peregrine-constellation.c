#include <stdint.h>
#include <stddef.h>
#include "interface/pconfig.h"

#include "interface/peregrine-constellation.h"
#include "packet.h"

// Initialize the library with the node address
pc_error_t pc_init(pc_handle_t *handle)
{
    if (handle == NULL)
    {
        return PC_ERROR_INVALID_HANDLE;
    }
    handle->node_address = 0;
    handle->retry_count = 0;
    handle->max_transmit_retries = pconfigMAX_RETRIES;
    handle->backoff_base_time_ms = pconfigBACKOFF_MAX_TIME_MS;
    handle->backoff_max_time_ms = pconfigBACKOFF_MAX_TIME_MS;
    handle->last_retry_time_ms = 0;
    return PC_SUCCESS;
}

pc_error_t pc_config_node_address(pc_handle_t *handle, uint8_t node_address)
{
    if (handle == NULL)
    {
        return PC_ERROR_INVALID_HANDLE;
    }
    handle->node_address = node_address;
    return PC_SUCCESS;
}

// Send a message
pc_error_t pc_send_message(pc_handle_t *handle, uint8_t dest_addr, const uint8_t *payload, size_t payload_length)
{
    if (handle == NULL)
    {
        printf("Invalid handle\n");
        return PC_ERROR_INVALID_HANDLE;
    }

    // TODO send to encoder
    return PC_SUCCESS;
}

// Process incoming data (should be called with received data)
void pc_process_incoming(pc_handle_t *handle, const uint16_t *data, size_t length)
{
    if (handle == NULL)
    {
        return;
    }

    // TODO send to decoder
}

// Update function to be called periodically to handle retries
void pc_task(pc_handle_t *handle)
{
    if (handle == NULL)
    {
        return;
    }

    // encoder task
    // decoder task
}