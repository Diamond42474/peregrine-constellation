#ifndef PACKET_SERIALIZER_H
#define PACKET_SERIALIZER_H

#include "packet.h"
#include "utils/circular_buffer.h"

int packet_serializer_serialize(const packet_t *packet, circular_buffer_t *output);

#endif // PACKET_SERIALIZER_H