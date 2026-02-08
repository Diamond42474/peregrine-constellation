#include "decoding/decoder.h"

static void _default_process_byte(unsigned char byte)
{

}

static void _default_process_bit(bool bit)
{

}

static void _default_process_packet(packet_t *packet)
{

}

static void (*byte_processor)(unsigned char) = _default_process_byte;
static void (*bit_processor)(bool) = _default_process_bit;
static void (*packet_processor)(packet_t *) = _default_process_packet;

void mock_decoder_reset(void)
{
    byte_processor = _default_process_byte;
    bit_processor = _default_process_bit;
    packet_processor = _default_process_packet;
}

void mock_decoder_set_byte_processor(void (*processor)(unsigned char))
{
    byte_processor = processor;
}

void mock_decoder_set_bit_processor(void (*processor)(bool))
{
    bit_processor = processor;
}

void mock_decoder_set_packet_processor(void (*processor)(packet_t *))
{
    packet_processor = processor;
}

int decoder_init(decoder_handle_t *handle)
{
    return 0;
}

int decoder_deinit(decoder_handle_t *handle)
{
    return 0;
}

int decoder_set_byte_decoder(decoder_handle_t *handle, byte_decoder_e type, void *byte_decoder_handle)
{
    return 0;
}

int decoder_set_bit_decoder(decoder_handle_t *handle, bit_decoder_e type, void *bit_decoder_handle)
{
    return 0;
}

int decoder_set_input_buffer_size(decoder_handle_t *handle, size_t size)
{
    return 0;
}

int decoder_set_output_buffer_size(decoder_handle_t *handle, size_t size)
{
    return 0;
}

int decoder_task(decoder_handle_t *handle)
{
    return 0;
}

int decoder_process_samples(decoder_handle_t *handle, const uint16_t *samples, size_t num_samples)
{
    return 0;
}

int decoder_process_bit(decoder_handle_t *handle, bool bit)
{
    bit_processor(bit);
    return 0;
}

int decoder_process_byte(decoder_handle_t *handle, unsigned char byte)
{
    byte_processor(byte);
    return 0;
}

int decoder_process_packet(decoder_handle_t *handle, packet_t *packet)
{
    packet_processor(packet);
    return 0;
}

bool decoder_has_packet(decoder_handle_t *handle)
{
    return false;
}

int decoder_get_packet(decoder_handle_t *handle, packet_t *packet)
{
    return 0;
}

bool decoder_busy(decoder_handle_t *handle)
{
    return false;
}
