#ifndef GOERTZEL_H
#define GOERTZEL_H

#include <stdint.h>
#include "circular_buffer.h"

int goertzel_compute_power(const uint16_t *samples, int num_samples, float target_freq, float sample_rate, float *power);
int goertzel_compute_power_circular_buff(const circular_buffer_t *cb, int num_samples, float target_freq, float sample_rate, float *power);

#endif // GOERTZEL_H