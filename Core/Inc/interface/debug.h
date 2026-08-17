#ifndef DEBUG_H
#define DEBUG_H

#include "pconfig.h"
#include <stdint.h>

#if pconfig_DEBUG_RECORDING_ENABLED
// If debug recording is enabled, the end user will
// need to implement this function to handle the recorded data.

/**
 * @brief Handles debug recording of ADC samples, filtered outputs, and metrics.
 *
 * @param sample The raw uint16_t ADC sample.
 * @param filtered_1200 The output of the 1200 Hz bandpass filter.
 * @param filtered_2200 The output of the 2200 Hz bandpass filter.
 * @param metric The computed metric for FSK decoding.
 *
 * @return error code: 0 = success, -1 = failure
 */
int debug_handle_recording(uint16_t sample, float filtered_1200, float filtered_2200, float metric);

#endif // pconfig_DEBUG_RECORDING_ENABLED

#endif // DEBUG_H