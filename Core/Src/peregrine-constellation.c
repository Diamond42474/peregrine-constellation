#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include "interface/pconfig.h"

#include "interface/peregrine-constellation.h"
#include "orchestrator.h"
#include "c-logger.h"

// Peregrine Constellation handle
typedef struct pc_handle
{
    message_callback_t message_callback;       // Callback for when a message is received and decoded
    orchestrator_handle_t orchestrator_handle; // Handle for the orchestrator which manages the modem and packet buffers
} pc_handle_t;

// Initialize the library with the node address
pc_handle_t *pc_init(message_callback_t callback)
{
    pc_handle_t *handle = malloc(sizeof(pc_handle_t));
    if (!handle)
    {
        LOG_ERROR("Failed to allocate memory for Peregrine Constellation handle");
        return NULL;
    }

    handle->message_callback = callback;

    if (!handle->message_callback)
    {
        LOG_ERROR("Message callback is NULL");
        goto failed;
    }

    if (orchestrator_init(&handle->orchestrator_handle, callback))
    {
        LOG_ERROR("Failed to initialize orchestrator");
        goto failed;
    }

    return handle;
failed:
    free(handle);
    return NULL;
}

// Send a message
pc_error_e pc_send_message(pc_handle_t *handle, uint8_t dest_addr, const uint8_t *payload, size_t payload_length)
{
    if (handle == NULL)
    {
        LOG_ERROR("Handle is NULL");
        return PC_ERROR_INVALID_HANDLE;
    }

    if (orchestrator_send(&handle->orchestrator_handle, payload, payload_length, dest_addr))
    {
        LOG_ERROR("Failed to send message through orchestrator");
        return PC_ERROR_INVALID_HANDLE;
    }

    return PC_SUCCESS;
}

// Update function to be called periodically to handle retries
void pc_task(pc_handle_t *handle)
{
    if (handle == NULL)
    {
        return;
    }

    orchestrator_task(&handle->orchestrator_handle);
}