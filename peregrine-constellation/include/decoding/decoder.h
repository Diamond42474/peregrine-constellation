#ifndef DECODER_H
#define DECODER_H

typedef enum {
    MESSAGE_DECODER_NONE,
    MESSAGE_DECODER_COBS,
} message_decoder_e;

typedef enum {
    BYTE_DECODER_NONE,
    BYTE_DECODER_BIT_STUFFING,
} byte_decoder_e;

typedef enum {
    BIT_DECODER_NONE,
    BIT_DECODER_FSK,
} bit_decoder_e;

typedef struct {
    message_decoder_e message_decoder;
    byte_decoder_e byte_decoder;
    bit_decoder_e bit_decoder;
} decoder_handle_t;

int decoder_init(decoder_handle_t *handle);
int decoder_deinit(decoder_handle_t *handle);

int decoder_set_message_decoder(decoder_handle_t *handle, message_decoder_e type);
message_decoder_e decoder_get_message_decoder(decoder_handle_t *handle);
int decoder_set_byte_decoder(decoder_handle_t *handle, byte_decoder_e type);
byte_decoder_e decoder_get_byte_decoder(decoder_handle_t *handle);
int decoder_set_bit_decoder(decoder_handle_t *handle, bit_decoder_e type);
bit_decoder_e decoder_get_bit_decoder(decoder_handle_t *handle);



#endif // DECODER_H