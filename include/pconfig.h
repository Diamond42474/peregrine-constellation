#ifndef pconfig_H
#define pconfig_H

#define pconfigMAX_PACKET_SIZE 32 // Maximum packet size
#define pconfigMAX_PAYLOAD_SIZE 15 // Maximum payload size

#define pconfigRX_BUFFER_SIZE 256 // Size of the receive buffer
#define pconfigTX_BUFFER_SIZE 4096 // Size of the transmit buffer * pconfigMAX_PAYLOAD_SIZE

#define pconfigMAX_RETRIES 5 // Number of retries before giving up
#define pconfigBACKOFF_BASE_TIME_MS 100 // Base time to wait before retrying
#define pconfigBACKOFF_MAX_TIME_MS 1000 // Maximum time to wait before retrying

#endif // pconfig_H