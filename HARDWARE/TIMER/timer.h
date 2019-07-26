#ifndef __TIMER_H
#define __TIMER_H

#include "stm32f10x.h"


void TIM3_Time_Init(u16 arr,u16 psc);                             //TIM3定时中断初始化
void TIM3_PWM_Init(u16 arr,u16 psc);  
void TIM5_PWMIn_Init(u16 arr,u16 psc);                            //TIM5 PWM输入捕获初始化
void TIM8_In_Init(u16 arr,u16 psc);                               //TIM8 输入捕获初始化

void TIM8_PWMOut_Init(u16 arr,u16 psc,u16 duty);                  //TIM8 PWM输出初始化
void ControlPWMOut1(TIM_TypeDef* TIMx,u16 arr,u16 psc,u16 duty);  //TIMX 控制PWM输出

void TIM2_ExtCount_Init(void);                                    //TIM2 外部计数模式初始化

#endif
