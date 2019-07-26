#ifndef __GPIO_H
#define __GPIO_H

#include "stm32f0xx_hal.h"

#define  	MODULE_PWRKEY_H  		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET)
#define  	MODULE_PWRKEY_L  		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET)
#define  	MODULE_RST_H  			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET)
#define  	MODULE_RST_L  			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET) 
#define 	tangze_A_on 				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET)
#define 	tangze_A_off 				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET)
#define 	tangze_B_on 				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_SET)
#define 	tangze_B_off 				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET)
#define 	ACC_off 			    		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_RESET)
#define 	ACC_on 				    	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_SET)
#define 	motor_A_off 				HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_RESET)
#define 	motor_A_on 				HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_SET)
#define 	battery_B_off 			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9,GPIO_PIN_RESET)
#define 	battery_B_on 				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9,GPIO_PIN_SET)
#define 	gsm_led_off 				HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_RESET)
#define 	gsm_led_on 				HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_SET)
#define 	read_key_det 			HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4)
#define 	read_acc_det 				HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3)
#define  	gps_led_on 				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET)
#define  	gps_led_off  				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET)

extern uint8_t flag_lock, f_motorlock,gsm_led_flag,gps_led_flag;
extern uint8_t flag_delay_lock, flag_delay_unlock, flag_batlock, flag_motorlock, flag_motorlock2;
extern uint8_t flag_delay500ms, flag_delay900ms, flag_delay10ms, flag_delay4s,flag_delay8s; 
extern uint8_t flag_tangze_unlock,flag_tangze_lock, flag_battery_lock;

void tangze_lock_bike(void);
void tangze_unlock_bike(void);
void motor_lock_bike(void);
void battery_lock(void);
void reset_system(void);
void close_electric_door(void);
void open_electric_door(void);
void gsm_led_process(void);
void gps_led_process(void);

#endif
