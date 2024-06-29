#include "peregrine-constellation.h"
#include "packet.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

static pc_handle_t handle;
static uint16_t node_address = 0x1234;

void print_hex_array(const void *object, size_t size) {
    const uint8_t *byte = (const uint8_t *)object;

    for (size_t i = 0; i < size; i++) {
        printf("0x%02X", byte[i]);
        if (i < size - 1) {
            printf(", ");
        }
    }
    printf("\n");
}

// Mock functions for demonstration
int send_function(const uint8_t *data, size_t size) {
    printf("Sending data: ");
    for (size_t i = 0; i < size; i++) {
        printf("0x%02X", data[i]);
        if (i < size - 1) {
            printf(", ");
        }
    }
    printf("\n");

    pc_process_incoming_data(&handle, data, size);
}

int line_busy_function(void) {
    return 0; // Return 0 if the line is not busy
}

void message_received_callback(const uint8_t *payload, size_t payload_length) {
    printf("Received message: ");
    for (size_t i = 0; i < payload_length; i++) {
        printf("%c", payload[i]);
    }
    printf("\n");
}

int main() {

    // Initialize Peregrine Constellation
    pc_init(&handle);
    pc_config_node_address(&handle, node_address);
    pc_config_received_callback(&handle, message_received_callback);
    pc_config_tx_func(&handle, send_function);
    pc_config_line_busy_func(&handle, line_busy_function);

    // Send a message
    const char *message = "Hello World!";
    if(pc_send_message(&handle, 0x1234, (const uint8_t *)message, strlen(message)))
    {
        printf("Error sending message\n");
    }

    // Simulate incoming data processing
    //uint8_t incoming_data[] = {0x01, 0x13, 0x34, 0x12, 0x34, 0x12, 0x01, 0x00, 0x05, 0x00, 0x00, 0x05, 0xF4, 0x01, 0x48, 0x65, 0x6C, 0x6C, 0x6F, 0xF5};
    //pc_process_incoming_data(&handle, incoming_data, sizeof(incoming_data));

    // Update (normally called periodically)
    pc_update(&handle, 1000);

   // mesh_packet_t packet;
    //initialize_packet(&packet, 1, 0x1234, 0x1234, 1, 5, (uint8_t *)message, strlen(message));

    //print_packet(&packet);
    //print_hex_array(&packet, sizeof(packet));

    return 0;
}
