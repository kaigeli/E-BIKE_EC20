#ifndef __SYSCONF_H
#define __SYSCONF_H	 
#include "misc.h"
#include "gpio.h"
#include "key.h"
#include "usart.h"
#include "can.h"

void NVIC_INIT(void);
void Hardware_Init(void);
void Variable_Init(void);

	 				    
#endif
