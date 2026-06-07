#ifndef pconfig_H
#define pconfig_H

#define pconfigDEVICE_ADDRESS (0x01) // 8-bit address for this device

#define pconfigMAX_PAYLOAD_SIZE 32 // Maximum payload size

#define pconfigRX_BUFFER_SIZE 128 // Number of packets that can be buffered for reception
#define pconfigTX_BUFFER_SIZE 128 // Number of packets that can be buffered for transmission

#define pconfigMAX_RETRIES 5            // Number of retries before giving up
#define pconfigBACKOFF_BASE_TIME_MS 100 // Base time to wait before retrying
#define pconfigBACKOFF_MAX_TIME_MS 1000 // Maximum time to wait before retrying

#define pconfigTTL 10 // Default Time To Live for packets, can be adjusted based on network size and requirements

// Default Encoder/Decoder Configurations
#define pconfigPREAMBLE_BYTE_1 0xAB
#define pconfigPREAMBLE_BYTE_2 0xBA

#define pconfigBAUD_RATE (250)
#define pconfigMODEM_FREQ_0 (1200)
#define pconfigMODEM_FREQ_1 (2200)

#define pconfigFSK_POWER_THRESHOLD (0.01f)      // Power threshold for FSK decoding (tune based on testing environment)
#define pconfigDECODER_BUFFER_SYMBOL_COUNT (10) // Multiple of symbol size
#define pconfigDECODER_OUTPUT_BUFFER_SIZE (10)  // Number of packets that can be buffered for the application to read

// Modem
#define pconfigMODEM_TX_BUFFER_SIZE (pconfigMAX_PAYLOAD_SIZE * 2) // Buffer for outgoing data to be transmitted, should be multiple of max payload size
#define pconfigPTT_DELAY_MS (500)                                 // Delay between setting PTT high and starting transmission to allow hardware to stabilize

#endif // pconfig_H