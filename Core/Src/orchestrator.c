#include "orchestrator.h"
#include "c-logger.h"
#include "interface/pconfig.h"
#include <string.h>

int orchestrator_init(orchestrator_handle_t *handle, rx_callback_t rx_callback)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Handle is NULL");
        return -1;
    }
    if (!rx_callback)
    {
        LOG_ERROR("RX callback is NULL");
        return -1;
    }

    memset(handle, 0, sizeof(orchestrator_handle_t));

    if (modem_init(&handle->modem, handle))
    {
        LOG_ERROR("Failed to init modem");
        return -1;
    }

    handle->rx_callback = rx_callback;

    if (circular_buffer_dynamic_init(&handle->rx_packet_buffer, sizeof(packet_t), pconfigRX_BUFFER_SIZE))
    {
        LOG_ERROR("Failed to init RX packet buffer");
        return -1;
    }

    if (circular_buffer_dynamic_init(&handle->tx_packet_buffer, sizeof(packet_t), pconfigTX_BUFFER_SIZE))
    {
        LOG_ERROR("Failed to init TX packet buffer");
        return -1;
    }

    return 0;
}

int orchestrator_send(orchestrator_handle_t *handle, const uint8_t *data, size_t len, uint8_t dest_addr)
{
    if (!handle)
    {
        LOG_ERROR("Handle is NULL");
        return -1;
    }
    if (!data)
    {
        LOG_ERROR("Data is NULL");
        return -1;
    }

    // Create data packet
    packet_t packet;
    memset(&packet, 0, sizeof(packet_t));
    packet.content.src_addr = pconfigDEVICE_ADDRESS;
    packet.content.dest_addr = dest_addr;
    packet.content.id = 0;   // TODO: Implement packet ID management
    packet.content.ttl = 0;  // TODO: Implement TTL management
    packet.content.type = 0; // TODO: Define packet types
    packet.content.payload_length = len > pconfigMAX_PAYLOAD_SIZE ? pconfigMAX_PAYLOAD_SIZE : len;
    memcpy(packet.content.payload, data, packet.content.payload_length);

    // Push packet to TX buffer for transmission by the modem task
    if (circular_buffer_push(&handle->tx_packet_buffer, &packet))
    {
        LOG_ERROR("Failed to push packet to TX buffer");
        return -1;
    }

    return 0;
}

int orchestrator_task(orchestrator_handle_t *handle)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Handle is NULL");
        return -1;
    }

    if (modem_task(&handle->modem))
    {
        LOG_ERROR("Modem task failed");
        return -1;
    }

    // TODO: Handle timing, retries, and other orchestrator-level tasks

    // PSUDO Sending
    if (circular_buffer_count(&handle->tx_packet_buffer) > 0)
    {

        if (!modem_busy(&handle->modem)) // Only send if modem is not busy
        {
            packet_t packet;
            if (circular_buffer_pop(&handle->tx_packet_buffer, &packet))
            {
                LOG_ERROR("Failed to pop packet from TX buffer");
                return -1;
            }

            if (modem_send_packet(&handle->modem, &packet))
            {
                LOG_ERROR("Failed to send packet through modem");
                return -1;
            }
        }
    }

    // PSUDO Receiving
    if (circular_buffer_count(&handle->rx_packet_buffer) > 0)
    {
        packet_t packet;
        if (circular_buffer_pop(&handle->rx_packet_buffer, &packet))
        {
            LOG_ERROR("Failed to pop packet from RX buffer");
            return -1;
        }

        // Call RX callback with packet payload
        if (handle->rx_callback)
        {
            handle->rx_callback(packet.content.payload, packet.content.payload_length, packet.content.src_addr);
        }
    }

    return 0;
}

/**
 * @brief Callback function for the modem when it decodes a packet
 *
 * @param handle Pointer to the orchestrator handle
 * @param packet Pointer to the decoded packet
 *
 * @return error code: 0 = success, -1 = failure
 */
int orchestrator_packet_callback(orchestrator_handle_t *handle, const packet_t *packet)
{
    if (!handle)
    {
        LOG_ERROR("Handle is NULL");
        return -1;
    }
    if (!packet)
    {
        LOG_ERROR("Packet is NULL");
        return -1;
    }

    if (circular_buffer_push(&handle->rx_packet_buffer, packet))
    {
        LOG_ERROR("Failed to push packet to RX buffer");
        return -1;
    }

    return 0;
}

int _send_packet(orchestrator_handle_t *handle, const packet_t *packet)
{
    if (!handle)
    {
        LOG_ERROR("Handle is NULL");
        return -1;
    }
    if (!packet)
    {
        LOG_ERROR("Packet is NULL");
        return -1;
    }

    return modem_send_packet(&handle->modem, packet);
}