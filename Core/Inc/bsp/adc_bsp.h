#ifndef ADC_BSP_H
#define ADC_BSP_H

#include "utils/circular_buffer.h"

int adc_bsp_init(int sample_rate);
int adc_bsp_task();
bool adc_bsp_data_available();
int adc_bsp_get_data(circular_buffer_t *buffer);

#endif // ADC_BSP_H