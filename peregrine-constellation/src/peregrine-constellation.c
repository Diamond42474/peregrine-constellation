#include <stdint.h>
#include <stddef.h>

#include "peregrine-constellation.h"
#include "../include/packet.h"

// Initialize the library with the node address
pc_error_t pc_init(pc_handle_t *handle)
{
    if (handle == NULL)
    {
        return PC_ERROR_INVALID_HANDLE;
    }
    handle->node_address = 0;
    handle->callback = NULL;
    handle->send_func = NULL;
    handle->line_busy_func = NULL;

    handle->retry_count = 0;
    handle->max_transmit_retries = pconfigMAX_RETRIES;
    handle->backoff_base_time_ms = pconfigBACKOFF_MAX_TIME_MS;
    handle->backoff_max_time_ms = pconfigBACKOFF_MAX_TIME_MS;
    handle->last_retry_time_ms = 0;
    return PC_SUCCESS;
}

pc_error_t pc_config_node_address(pc_handle_t *handle, uint16_t node_address)
{
    if (handle == NULL)
    {
        return PC_ERROR_INVALID_HANDLE;
    }
    handle->node_address = node_address;
    return PC_SUCCESS;

}

pc_error_t pc_config_received_callback(pc_handle_t *handle, pc_message_received_callback_t callback)
{
    if (handle == NULL)
    {
        return PC_ERROR_INVALID_HANDLE;
    }
    handle->callback = callback;
    return PC_SUCCESS;

}

pc_error_t pc_config_tx_func(pc_handle_t *handle, pc_send_func send_func)
{
    if (handle == NULL)
    {
        return PC_ERROR_INVALID_HANDLE;
    }
    handle->send_func = send_func;
    return PC_SUCCESS;
}

pc_error_t pc_config_line_busy_func(pc_handle_t *handle, pc_line_busy_func line_busy_func)
{
    if (handle == NULL)
    {
        return PC_ERROR_INVALID_HANDLE;
    }
    handle->line_busy_func = line_busy_func;
    return PC_SUCCESS;
}

// Send a message
pc_error_t pc_send_message(pc_handle_t *handle, uint16_t dest_addr, const uint8_t *payload, size_t payload_length)
{
    if (handle == NULL)
    {
        printf("Invalid handle\n");
        return PC_ERROR_INVALID_HANDLE;
    }
    if (handle->send_func == NULL)
    {
        return PC_ERROR_SEND_FUNCTION_NOT_SET;
    }

    packet_t packet;
    initialize_packet(&packet, 0, handle->node_address, dest_addr, 0, 10, payload, payload_length);
    handle->send_func((uint8_t *)&packet, sizeof(packet_t));
    return PC_SUCCESS;

}

// Process incoming data (should be called with received data)
void pc_process_incoming_data(pc_handle_t *handle, const uint8_t *data, size_t length)
{
    if (handle == NULL)
    {
        return;
    }
    if (length < calculate_min_size())
    {
        return;
    }
    packet_t *packet = (packet_t *)data;
    if (packet->content.dest_addr != handle->node_address)
    {
        return;
    }
    if (packet->content.payload_crc != calculate_payload_crc(packet))
    {
        return;
    }
    if (handle->callback != NULL)
    {
        handle->callback(packet->content.payload, packet->content.payload_length);
    }
}

// Update function to be called periodically to handle retries
void pc_update(pc_handle_t *handle, uint32_t current_time_ms)
{
    if (handle == NULL)
    {
        return;
    }
    if (handle->retry_count < handle->max_transmit_retries && current_time_ms - handle->last_retry_time_ms >= handle->backoff_base_time_ms)
    {
        handle->retry_count++;
        handle->last_retry_time_ms = current_time_ms;
    }
}