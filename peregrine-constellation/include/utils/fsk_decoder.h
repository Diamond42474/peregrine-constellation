#ifndef FSK_DECODER_H
#define FSK_DECODER_H

#include <stddef.h>
#include <stdint.h>

typedef struct fsk_decoder_handle
{
    int baud_rate;                                                            // Baud rate for FSK decoding
    int sample_rate;                                                          // Sample rate of the ADC
    float power_threshold;                                                    // Power threshold for detecting bits
    void (*bit_callback)(int bit);                                            // Callback function to handle decoded bits
    int (*adc_init)(void);                                                    // Function to initialize ADC
    int (*adc_start)(void);                                                   // Function to start ADC sampling
    int (*adc_stop)(void);                                                    // Function to stop ADC sampling
    int (*adc_set_callback)(void (*callback)(size_t size));                   // Function to set ADC sample callback
    int (*adc_set_sample_rate)(int rate);                                     // Function to set ADC sample rate
    int (*adc_set_sample_size)(int size);                                     // Function to set ADC sample size
    int (*adc_get_samples)(uint16_t *buffer, size_t size, int *samples_read); // Function to get ADC samples
} fsk_decoder_handle_t;

int fsk_decoder_init(fsk_decoder_handle_t *handle);
int fsk_decoder_deinit(fsk_decoder_handle_t *handle);

int fsk_decoder_set_baud_rate(fsk_decoder_handle_t *handle, int _baud_rate);
int fsk_decoder_set_sample_rate(fsk_decoder_handle_t *handle, int _sample_rate);
int fsk_decoder_set_frequencies(fsk_decoder_handle_t *handle, float freq_0, float freq_1);
int fsk_decoder_set_power_threshold(fsk_decoder_handle_t *handle, float _threshold);
int fsk_decoder_start(fsk_decoder_handle_t *handle);
int fsk_decoder_stop(fsk_decoder_handle_t *handle);
int fsk_decoder_is_running(fsk_decoder_handle_t *handle);
int fsk_decoder_process(fsk_decoder_handle_t *handle);

#endif // FSK_DECODER_H