#ifndef ENCODER_H
#define ENCODER_H

#include <stddef.h>

typedef enum {
    ENCODER_TYPE_NONE,
    ENCODER_TYPE_COBS,
} encoder_type_e;

typedef struct {
    encoder_type_e type;
} encoder_handle_t;

int encoder_init(encoder_handle_t *handle);
int encoder_deinit(encoder_handle_t *handle);

int encoder_set_type(encoder_handle_t *handle, encoder_type_e type);
encoder_type_e encoder_get_type(encoder_handle_t *handle);

int encoder_encode(encoder_handle_t *handle, const unsigned char *input, size_t input_len, unsigned char *output, size_t output_len);

#endif // ENCODER_H