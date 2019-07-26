#include "delay.h"
				   
volatile unsigned int timenum;      //????volatile!!!  
volatile unsigned long time_delay; // ????,?????????  
//延时nus
//nus为要延时的us数.		    								   
void delay_us(uint32_t nus)
{		
	//SYSTICK分频--1us的系统时钟中断  
    if (SysTick_Config(HAL_RCC_GetHCLKFreq()/1000000))  
    {  
     
        while (1);  
    }  
    time_delay=nus;//读取定时时间 
    while(time_delay);  
    SysTick->CTRL=0x00; //关闭计数器
    SysTick->VAL =0X00; //清空计数器
}

void delay_ms(uint16_t nms)
{	 		  	  
//SYSTICK分频--1ms的系统时钟中断   
    if (SysTick_Config(HAL_RCC_GetHCLKFreq()/1000))  
    {  
     
        while (1);  
    }  
    time_delay=nms;//读取定时时间
    while(time_delay);  
    SysTick->CTRL=0x00; //关闭计数器
    SysTick->VAL =0X00; //清空计数器
}  








































