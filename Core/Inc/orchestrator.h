#ifndef ORCHESTRATOR_H
#define ORCHESTRATOR_H

#include "utils/circular_buffer.h"
#include "interface/pconfig.h"
#include "modem.h"

typedef struct
{
    modem_handle_t modem;

    circular_buffer_t rx_packet_buffer; // Inbound packets
    size_t rx_packet_buffer_size;

    circular_buffer_t tx_packet_buffer; // Outbound packets
    size_t tx_packet_buffer_size;

} orchestrator_handle_t;

int orchestrator_init(orchestrator_handle_t *handle);
int orchestrator_deinit(orchestrator_handle_t *handle);

int orchestrator_send(orchestrator_handle_t *handle, const uint8_t *data, size_t len);
int orchestrator_data_available(orchestrator_handle_t *handle);
int orchestrator_read(orchestrator_handle_t *handle, uint8_t *data, size_t len);

int orchestrator_task(orchestrator_handle_t *handle);

int orchestrator_packet_callback(orchestrator_handle_t *handle, const packet_t *packet);

#endif // ORCHESTRATOR_H