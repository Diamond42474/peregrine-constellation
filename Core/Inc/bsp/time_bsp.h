#ifndef TIME_BSP_H
#define TIME_BSP_H

#include <stdint.h>

int time_bsp_init();
uint64_t time_bsp_get_ms();
uint64_t time_bsp_get_us();

#endif // TIME_BSP_H