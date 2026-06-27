#ifndef ORCHESTRATOR_H
#define ORCHESTRATOR_H

#include "utils/circular_buffer.h"
#include "interface/pconfig.h"
#include "utils/time_utils.h"
#include "modem.h"

// Callback type for when a packet is received and decoded, allowing the application to process it
typedef void (*rx_callback_t)(const uint8_t *data, size_t len, uint8_t src_addr);

typedef struct orchestrator_handle
{
    modem_handle_t modem;      //< Modem handle for managing RX/TX timing, tones, PTT, and such
    rx_callback_t rx_callback; //< Callback for when a data packet is received and decoded for the application layer

    circular_buffer_t rx_packet_buffer; //< Inbound packets
    packet_t rx_packet_array[pconfigRX_BUFFER_SIZE];
    circular_buffer_t tx_packet_buffer; //< Outbound packets
    packet_t tx_packet_array[pconfigTX_BUFFER_SIZE];

    HAL_timer_t beacon_timer;
} orchestrator_handle_t;

int orchestrator_init(orchestrator_handle_t *handle, rx_callback_t rx_callback);

int orchestrator_send(orchestrator_handle_t *handle, const uint8_t *data, size_t len, uint8_t dest_addr);

int orchestrator_task(orchestrator_handle_t *handle);

int orchestrator_packet_callback(orchestrator_handle_t *handle, const packet_t *packet);

#endif // ORCHESTRATOR_H