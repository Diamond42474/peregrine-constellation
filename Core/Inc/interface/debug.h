#ifndef DEBUG_H
#define DEBUG_H

#include "pconfig.h"
#include <stdint.h>

#if pconfig_DEBUG_RECORDING_ENABLED
// If debug recording is enabled, the end user will
// need to implement this function to handle the recorded data.

/**
 * @brief Handles debug recording of a raw ADC sample.
 *
 * @param sample The raw uint16_t ADC sample.
 *
 * @return error code: 0 = success, -1 = failure
 */
int debug_handle_recording(uint16_t sample);

#endif // pconfig_DEBUG_RECORDING_ENABLED

#endif // DEBUG_H
