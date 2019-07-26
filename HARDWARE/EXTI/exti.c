#include "exti.h"

volatile unsigned long rotate_count = 0, mileage_count = 10000, shake_count = 0;
uint32_t diff_rotate, diff_mileage, diff_shake;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
			//里程触发
     	if (GPIO_Pin == GPIO_PIN_5) 
 	{
		mileage_count ++;
		if (mileage_count > MAXMILE)
			mileage_count = 0;
	 }
			//轮动触发
	 if (GPIO_Pin == GPIO_PIN_6)
	 {
		  rotate_count++;
		  if (rotate_count > MAXROTATE)
			rotate_count = 0;
	 }

	 if(GPIO_Pin == GPIO_PIN_8)
	 {
		shake_count++;
		if(shake_count > MAXSHAKE)
			shake_count = 0;
	 }
}	

 

 
