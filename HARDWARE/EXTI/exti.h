#ifndef __EXTI_H
#define __EXIT_H	 
#include "stm32f0xx_hal.h"

#define MAXROTATE   2147483648
#define MAXMILE     2147483648
#define MAXSHAKE 2147483648

extern uint32_t diff_rotate, diff_mileage,diff_shake;
extern volatile unsigned long rotate_count, mileage_count, shake_count;	

#endif

