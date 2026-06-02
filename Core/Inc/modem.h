#ifndef MODEM_H
#define MODEM_H

#include "decoding/decoder.h"
#include "decoding/fsk_decoder.h"
#include "decoding/byte_assembler.h"
#include "decoding/packet_decoder.h"
#include "utils/circular_buffer.h"
#include "utils/time_utils.h"
#include "utils/bit_unpacker.h"

typedef struct
{
    // Orchestrator ctx
    void *orchestrator_ctx;

    // Decoder components
    decoder_handle_t decoder;
    fsk_decoder_handle_t fsk_decoder;
    byte_assembler_handle_t byte_assembler;
    packet_decoder_t packet_decoder;

    HAL_timer_t symbol_timer;        //< Timer for ensuring symbols are transmitted at the correct baud rate
    HAL_timer_t ptt_timer;           //< Delay between setting PTT high and starting transmission to allow hardware to stabilize
    circular_buffer_t tx_buffer; ///< Buffer for outgoing data to be transmitted
    bool transmitting;           ///< Flag to indicate if the modem is currently transmitting

    bit_unpacker_t bit_unpacker; ///< Bit unpacker for converting byte stream to bits for transmission
} modem_handle_t;

int modem_init(modem_handle_t *handle, void *orchestrator_ctx);

int modem_send_raw(modem_handle_t *handle, circular_buffer_t *cb);
int modem_send_packet(modem_handle_t *handle, const packet_t *packet);
bool modem_rx_busy(modem_handle_t *handle); // actively receiving
bool modem_tx_busy(modem_handle_t *handle); // actively transmitting
bool modem_busy(modem_handle_t *handle);    // rx or tx busy
int modem_task(modem_handle_t *handle);

#endif // MODEM_H