#ifndef __KEY_H
#define __KEY_H	 
#include "stm32f10x.h"
#include "usart.h"



//#define USER2    GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_3)  //读取按键0
//#define TAMPER   GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13) //读取按键1
//#define USER1    GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)  //读取按键2 
//#define WK_UP    GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)  //读取按键3(WK_UP) 

#define KEY3       GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_8)  //读取拔码3
#define KEY4       GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_9)  //读取拔码4
#define KEY5       GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_10) //读取拔码5
#define KEY6       GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_9)  //读取拔码6
#define KEY1       GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_14) //读取拔码1
#define KEY2       GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_15) //读取拔码2



void Key_Init(void);    //按键IO初始化	
u8 scan_key(void);      //按键扫描
void Key_Process(void);  

#endif
