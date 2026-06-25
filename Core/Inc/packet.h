#ifndef PACKET_H
#define PACKET_H

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include "interface/pconfig.h"

#define PACKET_HEADER_SIZE                     \
    (                                          \
        sizeof(uint8_t) + /* src_addr */       \
        sizeof(uint8_t) + /* dest_addr */      \
        sizeof(uint8_t) + /* id */             \
        sizeof(uint8_t) + /* ttl/type byte */  \
        sizeof(uint8_t) + /* payload_length */ \
        sizeof(uint16_t)  /* crc */            \
    )

#define PACKET_SIZE          \
    (                        \
        PACKET_HEADER_SIZE + \
        pconfigMAX_PAYLOAD_SIZE)

typedef enum
{
    PACKET_TYPE_BEACON = 0x0, //< Broadcasts callsign to satisfy the FCC
    PACKET_TYPE_DATA = 0x1,   //< Application layer data
    PACKET_TYPE_ACK = 0x2,    //< Acknowledgement for data packet received
} packet_type_e;

typedef struct
{
    /**
     * @brief Metadata about the packet (e.g., timestamps, RSSI, etc.)
     */
    struct
    {

    } metadata;

    /**
     * @brief Actual sent/received packet content
     */
    struct
    {
        uint8_t src_addr;
        uint8_t dest_addr;
        uint8_t id;
        uint8_t ttl : 4;
        uint8_t type : 4;
        uint8_t payload_length;
        uint16_t crc;
        uint8_t payload[pconfigMAX_PAYLOAD_SIZE];
    } content;
} packet_t;

uint16_t calculate_crc(const packet_t *packet);
int initialize_packet(packet_t *packet, packet_type_e packet_type, uint16_t src_addr, uint16_t dest_addr, uint8_t id, const uint8_t *payload, size_t payload_length);
void print_packet(const packet_t *packet);
#endif // PACKET_H
