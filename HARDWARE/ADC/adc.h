#ifndef __ADC_H
#define __ADC_H	

#include "stm32f0xx_hal.h"

extern uint32_t adc_val[64];

uint16_t get_bat_vol(void);

#endif 
