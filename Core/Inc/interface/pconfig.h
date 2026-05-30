#ifndef pconfig_H
#define pconfig_H

#define pconfigMAX_PAYLOAD_SIZE 32 // Maximum payload size

#define pconfigRX_BUFFER_SIZE 256  // Size of the receive buffer
#define pconfigTX_BUFFER_SIZE 4096 // Size of the transmit buffer * pconfigMAX_PAYLOAD_SIZE

#define pconfigMAX_RETRIES 5            // Number of retries before giving up
#define pconfigBACKOFF_BASE_TIME_MS 100 // Base time to wait before retrying
#define pconfigBACKOFF_MAX_TIME_MS 1000 // Maximum time to wait before retrying

// Default Encoder/Decoder Configurations
#define pconfigPREAMBLE_BYTE_1 0xAB
#define pconfigPREAMBLE_BYTE_2 0xBA

#define pconfigBAUD_RATE (250)
#define pconfigMODEM_FREQ_0 (1200)
#define pconfigMODEM_FREQ_1 (2200)

#define pconfigFSK_POWER_THRESHOLD (0.01f)     // Power threshold for FSK decoding (tune based on testing environment)
#define pconfigDECODER_BUFFER_SYMBOL_COUNT (3) // Multiple of symbol size
#define pconfigDECODER_OUTPUT_BUFFER_SIZE (10) // Number of packets that can be buffered for the application to read

// Modem
#define pconfigMODEM_TX_BUFFER_SIZE (pconfigMAX_PAYLOAD_SIZE * 2) // Buffer for outgoing data to be transmitted, should be multiple of max payload size

#endif // pconfig_H