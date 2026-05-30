#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <stdint.h>
#include <stdbool.h>

#define ONE_MS (1000)
#define ONE_SECOND (ONE_MS * 1000)

typedef struct 
{
    uint64_t duration_us;
    uint64_t finish_time_us;
} timer_t;

void time_utils_start(timer_t *timer, uint64_t duration_us);
bool time_utils_done(timer_t *timer);
void time_utils_reset(timer_t *timer);

#endif // TIME_UTILS_H