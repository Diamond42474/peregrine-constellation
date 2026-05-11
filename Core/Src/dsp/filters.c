#include "dsp/filters.h"

void design_bandpass_biquad(float lowcut, float highcut, float fs, biquad_coeffs_t *out)
{
    // Center frequency and bandwidth
    float fc = sqrtf(lowcut * highcut);
    float bw = highcut - lowcut;

    // Quality factor
    float Q = fc / bw;

    float omega = 2.0f * M_PI * fc / fs;
    float alpha = sinf(omega) / (2.0f * Q);
    float cosw = cosf(omega);

    // Bandpass (constant skirt gain, peak gain = Q)
    float b0 = alpha;
    float b1 = 0.0f;
    float b2 = -alpha;
    float a0 = 1.0f + alpha;
    float a1 = -2.0f * cosw;
    float a2 = 1.0f - alpha;

    // Normalize so a0 = 1
    out->b0 = b0 / a0;
    out->b1 = b1 / a0;
    out->b2 = b2 / a0;
    out->a1 = a1 / a0;
    out->a2 = a2 / a0;
}

void init_bandpass_4th(float lowcut, float highcut, float fs, biquad_t *s1, biquad_t *s2)
{
    biquad_coeffs_t c;

    // Use same design twice (simple Butterworth-like cascade)
    design_bandpass_biquad(lowcut, highcut, fs, &c);

    s1->c = c;
    s1->x1 = s1->x2 = s1->y1 = s1->y2 = 0.0f;

    s2->c = c;
    s2->x1 = s2->x2 = s2->y1 = s2->y2 = 0.0f;
}

/** Example usage:

biquad_t bp1200_1, bp1200_2;
biquad_t bp2200_1, bp2200_2;

void filters_init()
{
    init_bandpass_4th(1000.0f, 1400.0f, 70400.0f, &bp1200_1, &bp1200_2);
    init_bandpass_4th(2000.0f, 2400.0f, 70400.0f, &bp2200_1, &bp2200_2);
}
 */

// ==== Envelope metric ====

void env_metric_init(env_metric_t *s, float sample_rate, float tau_seconds)
{
    float dt = 1.0f / sample_rate;
    s->alpha = dt / (tau_seconds + dt);

    s->env1200 = 0.0f;
    s->env2200 = 0.0f;
}

/* Example usage:
env_metric_t em;
env_metric_init(&em, 70400.0f, 0.002f); // 2 ms
*/