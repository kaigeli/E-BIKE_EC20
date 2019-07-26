#include "voice.h"
#include "IoT_Hub.h"
#include "gpio.h"

#define VOICE_DATA_H HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET)
#define VOICE_DATA_L HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET)
#define VOICE_RST_H  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET)
#define VOICE_RST_L  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET)

extern TIM_HandleTypeDef htim1;
uint8_t voice_times;
uint8_t voice_pluse;

void delay_us(uint16_t time)
{    
   uint16_t i=0;  
   while(time--)
   {
      i=10;  //自己定义
      while(i--) ;    
   }
}

//语音播放
void voice_play(uint8_t plusenum, int8_t times)
{
//	uint8_t i;

//	Logln(D_INFO,"pluse=%d,times=%d",plusenum,times);
	voice_pluse = plusenum;
	voice_times = times;
}
void voice_process(void)
{
	uint8_t i;

	while(voice_times)
	{
		if (!HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7))
		{
			Logln(D_INFO, "voice IC busy");
			return;
		}
			
		VOICE_RST_H;
		delay_us(660);
		VOICE_RST_L;
		delay_us(660);

		for (i = 0; i < voice_pluse; i++)
		{
			VOICE_DATA_H;
			delay_us(330);
			VOICE_DATA_L;
			delay_us(330);
		}
		HAL_Delay(200);
		voice_times--;
	}
}

//定时器轮询实现延时n_us,
/*void delay_us(uint32_t n_us)  
{  
	uint16_t differ = 0xffff - n_us - 5;
	
	HAL_TIM_Base_Start(&htim1);
	__HAL_TIM_SET_COUNTER(&htim1,differ);
	while(differ < 0xffff-5)
	{
	 	differ = __HAL_TIM_GET_COUNTER(&htim1);
	}
	HAL_TIM_Base_Stop(&htim1);
}  
*/
