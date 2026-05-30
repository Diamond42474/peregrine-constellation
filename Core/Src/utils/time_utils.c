#include "utils/time_utils.h"
#include "bsp/time_bsp.h"

/**
 * @brief Starts a timer with the specified duration in microseconds.
 *
 * @param timer Pointer to the timer to start.
 * @param duration_us Duration of the timer in microseconds.
 */
void time_utils_start(timer_t *timer, uint64_t duration_us)
{
    timer->duration_us = duration_us;
    timer->finish_time_us = time_bsp_get_us() + duration_us;
}

/**
 * @brief Checks if the timer has completed.
 *
 * @param timer Pointer to the timer to check.
 * @return true if the timer has completed, false otherwise.
 */
bool time_utils_done(timer_t *timer)
{
    uint64_t now_us = time_bsp_get_us();
    return now_us >= timer->finish_time_us;
}

/**
 * @brief Resets the timer to start counting from the current time with the same duration.
 *
 * @param timer Pointer to the timer to reset.
 */
void time_utils_reset(timer_t *timer)
{
    timer->finish_time_us = time_bsp_get_us() + timer->duration_us;
}