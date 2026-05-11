#ifndef DSPS_FILTERS_H
#define DSPS_FILTERS_H

#include <math.h>
#include <stdint.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct
{
    float b0, b1, b2;
    float a1, a2; // a0 = 1
} biquad_coeffs_t;

typedef struct
{
    float x1, x2;
    float y1, y2;
    biquad_coeffs_t c;
} biquad_t;

void design_bandpass_biquad(float lowcut, float highcut, float fs, biquad_coeffs_t *out);
void init_bandpass_4th(float lowcut, float highcut, float fs, biquad_t *s1, biquad_t *s2);

static inline float biquad_process(biquad_t *f, float x)
{
    float y =
        f->c.b0 * x +
        f->c.b1 * f->x1 +
        f->c.b2 * f->x2 -
        f->c.a1 * f->y1 -
        f->c.a2 * f->y2;

    f->x2 = f->x1;
    f->x1 = x;
    f->y2 = f->y1;
    f->y1 = y;

    return y;
}

static inline float process_sos(biquad_t *s, int n, float x)
{
    for (int i = 0; i < n; i++) {
        x = biquad_process(&s[i], x);
    }
    return x;
}

// ==== Envelope metric ====
typedef struct
{
    float env1200;
    float env2200;
    float alpha; // smoothing factor (0–1)
} env_metric_t;

void env_metric_init(env_metric_t *s, float sample_rate, float tau_seconds);

static inline float env_metric_process(env_metric_t *s, float y1200, float y2200)
{
    // Envelope (abs + 1st-order lowpass)
    s->env1200 += s->alpha * (fabsf(y1200) - s->env1200);
    s->env2200 += s->alpha * (fabsf(y2200) - s->env2200);

    // Metric (soft decision)
    return s->env2200 - s->env1200;
}

#endif // DSP_FILTERS_H