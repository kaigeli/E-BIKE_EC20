#ifndef __VOICE_H
#define __VOICE_H

#include "stm32f0xx_hal.h"
void voice_play(uint8_t plusenum, int8_t times);
void voice_process(void);

#endif
