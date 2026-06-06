#include "peregrine-constellation.h"

void message_callback(const uint8_t *data, size_t len, uint8_t src_addr)
{
    // Print received message
    printf("Received message from 0x%02X: ", src_addr);
    for (size_t i = 0; i < len; i++)
    {
        printf("%02X ", data[i]);
    }
    printf("\n");
}

int main()
{
    pc_handle_t *handle = pc_init(message_callback);
    if (!handle)
    {
        return -1;
    }
    
    while (1)
    {
        pc_task(handle);
    }
}