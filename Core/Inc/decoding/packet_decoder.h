#ifndef PACKET_DECODER_H
#define PACKET_DECODER_H

#include "../packet.h"

typedef struct
{
    void *ctx;                                                  ///< Pointer to the decoder context
    uint8_t packet_buffer[sizeof(((packet_t *)NULL)->content)]; ///< Buffer to hold incoming packet data
    size_t packet_buffer_index;                                 ///< Current index in the packet buffer
    packet_t current_packet;                                    ///< Current packet being processed

    enum
    {
        PACKET_DECODER_STATE_WAITING_FOR_HEADER,
        PACKET_DECODER_STATE_WAITING_FOR_PAYLOAD,
    } state;
} packet_decoder_t;

int packet_decoder_init(packet_decoder_t *handle, void *ctx);
int packet_decoder_deinit(packet_decoder_t *handle);
int packet_decoder_process_byte(packet_decoder_t *handle, unsigned char byte);
int packet_decoder_reset(packet_decoder_t *handle);

#endif // PACKET_DECODER_H