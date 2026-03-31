#include "utils/fsk_utils.h"
#include <math.h>

#define OVERSAMPLING_FACTOR (3)
#define MIN_SAMPLES_PER_BIT (32)

/**
 * @brief Calculate the recommended sample rate for FSK modulation based on the given frequencies and baud rate.
 *
 * @param f1 The first frequency (Hz)
 * @param f2 The second frequency (Hz)
 * @param baud The baud rate (symbols per second)
 *
 * @return The recommended sample rate in Hz
 */
double calculate_sample_rate(double f1, double f2, double baud)
{
    double fmax = (f1 > f2) ? f1 : f2;

    // Minimum samples per symbol from Nyquist
    double n_nyquist = ceil((4.0 * fmax) / baud);

    double N = (n_nyquist > MIN_SAMPLES_PER_BIT) ? n_nyquist : MIN_SAMPLES_PER_BIT;

    return baud * N * OVERSAMPLING_FACTOR;
}
