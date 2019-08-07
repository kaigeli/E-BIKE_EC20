#include "adc.h"
float adc_param=0;
 
uint16_t get_bat_vol(void)
{
   	uint8_t t;
   	uint32_t ADvol = 0, batvol_10mv;
	float bat_vol,ave_vol;
	 
   	ADvol=0;
	for(t = 0; t < 64; t++) 
	{              	//设置采集次数
      		ADvol += adc_val[t];     						//采集到的AD值 
   	}
	 ave_vol = ADvol / 64;
	 bat_vol = (ave_vol*adc_param)/4095;   		//得出电池电压值
	 batvol_10mv = (uint32_t)(bat_vol*100);	//+50; //电压放大100倍	//加0.5v补偿二极管压降误差

         return batvol_10mv;                   
}

