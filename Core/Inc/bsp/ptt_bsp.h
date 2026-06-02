#ifndef PTT_BSP_H
#define PTT_BSP_H

#include <stdbool.h>

int ptt_bsp_init();
int ptt_bsp_task();
int ptt_bsp_set_ptt(bool active);

#endif // PTT_BSP_H